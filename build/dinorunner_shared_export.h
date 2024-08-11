
#ifndef DINORUNNER_SHARED_EXPORT_H
#define DINORUNNER_SHARED_EXPORT_H

#ifdef DINORUNNER_SHARED_STATIC_DEFINE
#  define DINORUNNER_SHARED_EXPORT
#  define DINORUNNER_SHARED_NO_EXPORT
#else
#  ifndef DINORUNNER_SHARED_EXPORT
#    ifdef dinorunner_shared_EXPORTS
        /* We are building this library */
#      define DINORUNNER_SHARED_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define DINORUNNER_SHARED_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef DINORUNNER_SHARED_NO_EXPORT
#    define DINORUNNER_SHARED_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef DINORUNNER_SHARED_DEPRECATED
#  define DINORUNNER_SHARED_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef DINORUNNER_SHARED_DEPRECATED_EXPORT
#  define DINORUNNER_SHARED_DEPRECATED_EXPORT DINORUNNER_SHARED_EXPORT DINORUNNER_SHARED_DEPRECATED
#endif

#ifndef DINORUNNER_SHARED_DEPRECATED_NO_EXPORT
#  define DINORUNNER_SHARED_DEPRECATED_NO_EXPORT DINORUNNER_SHARED_NO_EXPORT DINORUNNER_SHARED_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef DINORUNNER_SHARED_NO_DEPRECATED
#    define DINORUNNER_SHARED_NO_DEPRECATED
#  endif
#endif

#endif /* DINORUNNER_SHARED_EXPORT_H */
