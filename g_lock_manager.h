#ifndef _G_LOCK_MANAGER_H
#define _G_LOCK_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <glib.h>

typedef struct {
  GList *locks;
  GRWLock manager_rw_lock;
  uint32_t lock_index;
  bool allow_wrong_order;
  bool debug;
} GLockManager;

enum g_lock_type {
  G_LOCK_MUTEX = 0, /*<< The simplest lock - MUTEX */
  G_LOCK_RECURSIVE, /*<< A recursive mutex */
  G_LOCK_RW, /*<< A read/write mutex */
};

enum g_lock_action {
  G_LOCK_ACTION_BASIC=0,
  G_LOCK_ACTION_READ,
  G_LOCK_ACTION_WRITE,
};

typedef struct {
  GList *lock_list; /**< Locked indeces in a session */
} GLockSession;

struct g_lock_caller {
  char *caller; /*<< Caller function */
  uint32_t line; /*<< Caller function line number */
  time_t timestamp; /** Timestamp session was started */
  GLockSession *session;
};

struct g_lock_stats {
  int count; /*<< Number of callers waiting/using lock */
  GList *call_list; /**< List of callers. Acts like a queue */
};

typedef struct {
  union {
    GMutex mutex;
    GRecMutex rec_mutex;
    GRWLock rw_mutex;
  } _lock;
  char *name;
  GMutex stats_lock;
  enum g_lock_type type;
  struct g_lock_stats stats;
  uint32_t index;
} GLock;


GLock *g_lock_create(
  const char *lock_name,
  enum g_lock_type type
  );
#define g_lock_create_mutex(name) g_lock_create(name, G_LOCK_MUTEX)
#define g_lock_create_recursive(name) g_lock_create(name, G_LOCK_RECURSIVE)
#define g_lock_create_rw(name) g_lock_create(name, G_LOCK_RW)

#define g_lock_start(session, lock) \
  _g_lock_start(session, lock, G_LOCK_ACTION_BASIC, __FUNCTION__, __LINE__)
#define g_lock_start_read(session, lock) \
  _g_lock_start(session, lock, G_LOCK_ACTION_READ, __FUNCTION__, __LINE__)
#define g_lock_start_write(session, lock) \
  _g_lock_start(session, lock, G_LOCK_ACTION_WRITE, __FUNCTION__, __LINE__)
bool _g_lock_start(
  GLockSession *session,
  GLock *lock,
  enum g_lock_action action,
  const char *caller_func,
  uint32_t caller_line
  );

#define g_lock_end(session, lock) \
  _g_lock_end(session, lock, G_LOCK_ACTION_BASIC, __FUNCTION__, __LINE__)
#define g_lock_end_read(session, lock) \
  _g_lock_end(session, lock, G_LOCK_ACTION_READ, __FUNCTION__, __LINE__)
#define g_lock_end_write(session, lock) \
  _g_lock_end(session, lock, G_LOCK_ACTION_WRITE, __FUNCTION__, __LINE__)
void _g_lock_end(
  GLockSession *session,
  GLock *lock,
  enum g_lock_action action,
  const char *caller_func,
  uint32_t caller_line
  );

void g_lock_free_all();
void g_lock_free(GLock *lock);
void g_lock_show_all();
char *g_lock_name_by_index(uint32_t index);

GLockSession *g_lock_session_new();
void g_lock_session_free(GLockSession *session);

#define G_LOCK_SESSION_START() \
  GLockSession *session = g_lock_session_new(); \
  if(!session) { \
    return; \
  }

#define G_LOCK_SESSION_END() \
  do { \
    g_lock_session_free(session); \
  } while(0)

void g_lock_manager_init();
void g_lock_manager_free();
void g_lock_manager_set_debug(bool debug);
void g_lock_manager_allow_wrong_order(bool allow);

#endif // _G_LOCK_MANAGER_H

