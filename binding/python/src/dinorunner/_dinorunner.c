/**
 * @file _dinorunner.c
 * @brief CPython extension: libdinorunner's engine, forwarding its callbacks to a Python object
 *
 * The engine calls the callbacks defined at the bottom of this file, through kCallbacks. They call
 * the methods of the platform object given to `Engine(...)`. An exception raised by such a method is
 * kept and raised again once the engine returns control, from the method that drove the engine.
 *
 * @copyright Copyright (C) 2026 - All Rights Reserved
 *  You may use, distribute and modify this code under the
 *  terms of the GPL license.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "dinorunner.h"

static PyObject* DinorunnerError = NULL;

typedef struct {
  PyObject_HEAD
  struct dinorunner_s state;
  PyObject* platform;
  /* First exception raised by a callback, raised again when the engine returns */
#if PY_VERSION_HEX >= 0x030C0000
  PyObject* error;
#else
  PyObject* error_type;
  PyObject* error;
  PyObject* error_traceback;
#endif
  unsigned long last_timestamp;
  int initialized;
  /* Set while the engine runs: callbacks must not drive the engine again */
  int busy;
} EngineObject;

static void engine_store_error(EngineObject* self) {
  if (self->error != NULL) {
    /* Keep the first exception, the following ones are consequences of it */
    PyErr_Clear();
    return;
  }
#if PY_VERSION_HEX >= 0x030C0000
  self->error = PyErr_GetRaisedException();
#else
  PyErr_Fetch(&self->error_type, &self->error, &self->error_traceback);
  PyErr_NormalizeException(&self->error_type, &self->error, &self->error_traceback);
#endif
}

/* Raises the stored exception, if any. Returns -1 when an exception was raised */
static int engine_raise_error(EngineObject* self) {
  if (self->error == NULL) {
    return 0;
  }
#if PY_VERSION_HEX >= 0x030C0000
  PyErr_SetRaisedException(self->error);
  self->error = NULL;
#else
  PyErr_Restore(self->error_type, self->error, self->error_traceback);
  self->error_type      = NULL;
  self->error           = NULL;
  self->error_traceback = NULL;
#endif
  return -1;
}

static int engine_enter(EngineObject* self) {
  if (!self->initialized) {
    PyErr_SetString(PyExc_RuntimeError, "the engine is not initialized");
    return -1;
  }
  if (self->busy) {
    PyErr_SetString(PyExc_RuntimeError, "the engine cannot be driven from within one of its callbacks");
    return -1;
  }
  self->busy = 1;
  return 0;
}

/* Ends an engine call: raises a callback's exception instead of returning `result` */
static PyObject* engine_leave(EngineObject* self, PyObject* result) {
  self->busy = 0;
  if (engine_raise_error(self) < 0) {
    Py_XDECREF(result);
    return NULL;
  }
  return result;
}

/* Callbacks of the C engine, defined at the bottom of this file */
static unsigned long on_timestamp(void* user_data);
static unsigned char on_clear_canvas(void* user_data);
static unsigned char on_draw(enum dinorunner_sprite_e sprite, const struct pos_s* pos, unsigned char opacity,
                             void* user_data);
static unsigned char on_play_sound(enum dinorunner_sound_e sound, void* user_data);
static unsigned char on_vibrate(unsigned duration, void* user_data);
static unsigned char on_read_high_score(unsigned long* high_score, void* user_data);
static unsigned char on_write_high_score(unsigned long high_score, void* user_data);
static unsigned char on_log(const char* message, void* user_data);

static const struct dinorunner_callbacks_s kCallbacks = {
    .gettimestamp   = on_timestamp,
    .clearcanvas    = on_clear_canvas,
    .draw           = on_draw,
    .playsound      = on_play_sound,
    .vibrate        = on_vibrate,
    .readhighscore  = on_read_high_score,
    .writehighscore = on_write_high_score,
    .log            = on_log,
};

static int engine_init(EngineObject* self, PyObject* args, PyObject* kwargs) {
  static char* keywords[] = {"width", "height", "platform", NULL};
  unsigned int width, height;
  PyObject* platform;
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "IIO:Engine", keywords, &width, &height, &platform)) {
    return -1;
  }
  if (self->initialized || self->busy) {
    PyErr_SetString(PyExc_RuntimeError, "the engine is already initialized");
    return -1;
  }
  Py_INCREF(platform);
  Py_XSETREF(self->platform, platform);
  const struct dimension_s dimension = {.width = width, .height = height};
  self->busy                         = 1;
  const unsigned char result         = dinorunner_init(&self->state, &dimension, &kCallbacks, self);
  self->busy                         = 0;
  if (engine_raise_error(self) < 0) {
    return -1;
  }
  if (!result) {
    PyErr_SetString(DinorunnerError, "could not initialize the dinorunner engine");
    return -1;
  }
  self->initialized = 1;
  return 0;
}

static PyObject* engine_update(EngineObject* self, PyObject* Py_UNUSED(ignored)) {
  if (engine_enter(self) < 0) {
    return NULL;
  }
  if (!dinorunner_update(&self->state)) {
    self->busy = 0;
    if (engine_raise_error(self) == 0) {
      PyErr_SetString(DinorunnerError, "could not update the dinorunner engine");
    }
    return NULL;
  }
  return engine_leave(self, Py_NewRef(Py_None));
}

static PyObject* engine_key_up(EngineObject* self, PyObject* Py_UNUSED(ignored)) {
  if (engine_enter(self) < 0) {
    return NULL;
  }
  dinorunner_onkeyup(&self->state);
  return engine_leave(self, Py_NewRef(Py_None));
}

static PyObject* engine_key_down(EngineObject* self, PyObject* Py_UNUSED(ignored)) {
  if (engine_enter(self) < 0) {
    return NULL;
  }
  dinorunner_onkeydown(&self->state);
  return engine_leave(self, Py_NewRef(Py_None));
}

static PyObject* engine_key_none(EngineObject* self, PyObject* Py_UNUSED(ignored)) {
  if (engine_enter(self) < 0) {
    return NULL;
  }
  dinorunner_onkeynone(&self->state);
  return engine_leave(self, Py_NewRef(Py_None));
}

/* Reading the state is allowed from within callbacks */
static PyObject* engine_get_inverted(EngineObject* self, void* Py_UNUSED(closure)) {
  unsigned char inverted = 0u;
  if (self->initialized) {
    dinorunner_isinverted(&self->state, &inverted);
  }
  return PyBool_FromLong(inverted);
}

static PyObject* engine_get_alive(EngineObject* self, void* Py_UNUSED(closure)) {
  unsigned char alive = 0u;
  if (self->initialized) {
    dinorunner_isalive(&self->state, &alive);
  }
  return PyBool_FromLong(alive);
}

static int engine_traverse(EngineObject* self, visitproc visit, void* arg) {
  Py_VISIT(Py_TYPE(self));
  Py_VISIT(self->platform);
  Py_VISIT(self->error);
#if PY_VERSION_HEX < 0x030C0000
  Py_VISIT(self->error_type);
  Py_VISIT(self->error_traceback);
#endif
  return 0;
}

static int engine_clear(EngineObject* self) {
  Py_CLEAR(self->platform);
  Py_CLEAR(self->error);
#if PY_VERSION_HEX < 0x030C0000
  Py_CLEAR(self->error_type);
  Py_CLEAR(self->error_traceback);
#endif
  return 0;
}

static void engine_dealloc(EngineObject* self) {
  PyTypeObject* type = Py_TYPE(self);
  PyObject_GC_UnTrack(self);
  engine_clear(self);
  type->tp_free((PyObject*)self);
  Py_DECREF(type);
}

static PyMethodDef engine_methods[] = {
    {"update", (PyCFunction)engine_update, METH_NOARGS, "Advance the game by one frame and draw it."},
    {"key_up", (PyCFunction)engine_key_up, METH_NOARGS, "The jump key was pressed."},
    {"key_down", (PyCFunction)engine_key_down, METH_NOARGS, "The duck key was pressed."},
    {"key_none", (PyCFunction)engine_key_none, METH_NOARGS, "All keys were released."},
    {NULL, NULL, 0, NULL},
};

static PyGetSetDef engine_getset[] = {
    {"inverted", (getter)engine_get_inverted, NULL, "Whether night mode is active.", NULL},
    {"alive", (getter)engine_get_alive, NULL, "Whether the intro is over and the game is running.", NULL},
    {NULL, NULL, NULL, NULL, NULL},
};

static PyType_Slot engine_slots[] = {
    {Py_tp_doc, "Engine(width, height, platform): libdinorunner's game state, drawing through platform."},
    {Py_tp_init, engine_init},
    {Py_tp_dealloc, engine_dealloc},
    {Py_tp_traverse, engine_traverse},
    {Py_tp_clear, engine_clear},
    {Py_tp_methods, engine_methods},
    {Py_tp_getset, engine_getset},
    {0, NULL},
};

static PyType_Spec engine_spec = {
    .name      = "dinorunner._dinorunner.Engine",
    .basicsize = sizeof(EngineObject),
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .slots     = engine_slots,
};

static PyObject* module_version(PyObject* Py_UNUSED(module), PyObject* Py_UNUSED(ignored)) {
  struct version_s version = {0u, 0u, 0u};
  dinorunner_getversion(&version);
  return Py_BuildValue("(BBB)", version.major, version.minor, version.patch);
}

static PyObject* module_seed(PyObject* Py_UNUSED(module), PyObject* arg) {
  const unsigned long seed = PyLong_AsUnsignedLong(arg);
  if (seed == (unsigned long)-1 && PyErr_Occurred()) {
    return NULL;
  }
  if (seed > 0xFFFFu) {
    PyErr_SetString(PyExc_ValueError, "the seed must be in range(0, 65536)");
    return NULL;
  }
  dinorunner_seed((unsigned short)seed);
  Py_RETURN_NONE;
}

static PyMethodDef module_methods[] = {
    {"version", module_version, METH_NOARGS, "Version of libdinorunner as (major, minor, patch)."},
    {"seed", module_seed, METH_O, "Reseed the engine's pseudo-random generator, shared by all games."},
    {NULL, NULL, 0, NULL},
};

static struct PyModuleDef module_definition = {
    PyModuleDef_HEAD_INIT,
    .m_name    = "dinorunner._dinorunner",
    .m_doc     = "libdinorunner's C engine. Use the dinorunner package instead.",
    .m_size    = -1,
    .m_methods = module_methods,
};

PyMODINIT_FUNC PyInit__dinorunner(void) {
  PyObject* module = PyModule_Create(&module_definition);
  if (module == NULL) {
    return NULL;
  }
  PyObject* engine_type = PyType_FromSpec(&engine_spec);
  if (PyModule_AddObject(module, "Engine", engine_type) < 0) {
    Py_XDECREF(engine_type);
    Py_DECREF(module);
    return NULL;
  }
  DinorunnerError = PyErr_NewExceptionWithDoc("dinorunner.DinorunnerError",
                                              "The dinorunner engine reported a failure.", PyExc_RuntimeError, NULL);
  if (DinorunnerError == NULL || PyModule_AddObject(module, "DinorunnerError", Py_NewRef(DinorunnerError)) < 0 ||
      PyModule_AddIntConstant(module, "SPRITE_COUNT", DINORUNNER_SPRITE_GAMEOVER + 1) < 0 ||
      PyModule_AddIntConstant(module, "SOUND_COUNT", DINORUNNER_SOUND_HIT + 1) < 0) {
    Py_DECREF(module);
    return NULL;
  }
  return module;
}

/* ---- Callbacks of the C engine ---- */

/* Calls `platform.<method>(*args)`, `args` is stolen. None and truthy results count as success */
static unsigned char call_platform(EngineObject* self, const char* method, PyObject* args) {
  if (args == NULL || self->error != NULL || self->platform == NULL) {
    if (args == NULL) {
      engine_store_error(self);
    }
    Py_XDECREF(args);
    return 0u;
  }
  PyObject* callable = PyObject_GetAttrString(self->platform, method);
  PyObject* result   = callable != NULL ? PyObject_Call(callable, args, NULL) : NULL;
  Py_XDECREF(callable);
  Py_DECREF(args);
  if (result == NULL) {
    engine_store_error(self);
    return 0u;
  }
  const int success = result == Py_None ? 1 : PyObject_IsTrue(result);
  Py_DECREF(result);
  if (success < 0) {
    engine_store_error(self);
    return 0u;
  }
  return success ? 1u : 0u;
}

static unsigned long on_timestamp(void* user_data) {
  EngineObject* self = (EngineObject*)user_data;
  if (self->error != NULL || self->platform == NULL) {
    return self->last_timestamp;
  }
  PyObject* result = PyObject_CallMethod(self->platform, "timestamp", NULL);
  /* Integers, or floats truncated to whole milliseconds; strings are rejected */
  PyObject* number = NULL;
  if (result != NULL) {
    number = PyFloat_Check(result) ? PyNumber_Long(result) : PyNumber_Index(result);
  }
  Py_XDECREF(result);
  if (number == NULL) {
    engine_store_error(self);
    return self->last_timestamp;
  }
  const unsigned long long timestamp = PyLong_AsUnsignedLongLong(number);
  Py_DECREF(number);
  if (PyErr_Occurred()) {
    engine_store_error(self);
    return self->last_timestamp;
  }
  /* Truncated where long is 32 bits wide: the engine only uses differences */
  self->last_timestamp = (unsigned long)timestamp;
  return self->last_timestamp;
}

static unsigned char on_clear_canvas(void* user_data) {
  return call_platform((EngineObject*)user_data, "clear_canvas", PyTuple_New(0));
}

static unsigned char on_draw(enum dinorunner_sprite_e sprite, const struct pos_s* pos, unsigned char opacity,
                             void* user_data) {
  if (pos == NULL) {
    return 0u;
  }
  return call_platform((EngineObject*)user_data, "draw", Py_BuildValue("(iiiB)", (int)sprite, pos->x, pos->y, opacity));
}

static unsigned char on_play_sound(enum dinorunner_sound_e sound, void* user_data) {
  return call_platform((EngineObject*)user_data, "play_sound", Py_BuildValue("(i)", (int)sound));
}

static unsigned char on_vibrate(unsigned duration, void* user_data) {
  return call_platform((EngineObject*)user_data, "vibrate", Py_BuildValue("(I)", duration));
}

static unsigned char on_write_high_score(unsigned long high_score, void* user_data) {
  return call_platform((EngineObject*)user_data, "write_high_score", Py_BuildValue("(k)", high_score));
}

static unsigned char on_read_high_score(unsigned long* high_score, void* user_data) {
  EngineObject* self = (EngineObject*)user_data;
  if (high_score == NULL || self->error != NULL || self->platform == NULL) {
    return 0u;
  }
  PyObject* result = PyObject_CallMethod(self->platform, "read_high_score", NULL);
  if (result == NULL) {
    engine_store_error(self);
    return 0u;
  }
  /* None: no high score yet, which the engine expects as 0 */
  unsigned long value = 0ul;
  if (result != Py_None) {
    value = PyLong_AsUnsignedLong(result);
  }
  Py_DECREF(result);
  if (PyErr_Occurred()) {
    engine_store_error(self);
    return 0u;
  }
  *high_score = value;
  return 1u;
}

static unsigned char on_log(const char* message, void* user_data) {
  return call_platform((EngineObject*)user_data, "log", Py_BuildValue("(s)", message));
}
