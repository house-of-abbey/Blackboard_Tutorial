#ifndef _BWIFI_PROTO_H_
#define _BWIFI_PROTO_H_

#define BWIFI_VERSION 2

typedef enum {
  BWIFI_PACKET_TYPE_VERSION = 0,
  BWIFI_PACKET_TYPE_STATUS  = 1,
  BWIFI_PACKET_TYPE_AUTH    = 2,
  BWIFI_PACKET_TYPE_LOG     = 3,
  BWIFI_PACKET_TYPE_DATA    = 4,
  BWIFI_PACKET_TYPE_ADDRESS = 5,
  BWIFI_PACKET_TYPE_TIME    = 6,
} bwifi_packet_type_t;

typedef enum {
  BWIFI_LOG_LEVEL_INFO  = 1,
  BWIFI_LOG_LEVEL_WARN  = 2,
  BWIFI_LOG_LEVEL_ERROR = 3,
} bwifi_log_level_t;

/*
       +-----------------+
       |  Uninitialised  |
       +-----------------+
                |
                v
          +-----------+
          |  Booting  |
          +-----------+
                |
                v
        +----------------+
        |  Initialising  |
        +----------------+
                |
                v
        +----------------+
        | Auth Required  | <--------+
        +----------------+          |
                |                   |
                |                   |
                v                   |
          +------------+            |
          | Auth Sent  |    Connection Failed
          +------------+            |
                |                   |
                |                   |
                v                   |
          +-------------+           |
          |             | ----------+
          | Connecting  |
          |             | <---------+
          +-------------+           |
                |                   |
       Connection Successful   Disconnected
                v                   |
          +-----------+             |
          | Connected | ------------+
          +-----------+
*/
/** States of the state machine for the BWifi module. */
typedef enum {
  BWIFI_STATE_UNINITIALISED = 0,
  BWIFI_STATE_BOOTING       = 1,
  BWIFI_STATE_INITIALISING  = 2,
  BWIFI_STATE_AUTH_REQUIRED = 3,
  BWIFI_STATE_AUTH_SENT     = 4,
  BWIFI_STATE_CONNECTING    = 5,
  BWIFI_STATE_CONNECTED     = 6,
} bwifi_state_t;

/** Arcs of the state machine for the BWifi module. */
typedef enum {
  BWIFI_STATUS_INITIALISING          = 1,
  BWIFI_STATUS_AUTH_REQUIRED         = 2,
  BWIFI_STATUS_CONNECTING            = 3,
  BWIFI_STATUS_CONNECTION_FAILED     = 4,
  BWIFI_STATUS_CONNECTION_SUCCESSFUL = 5,
  BWIFI_STATUS_DISCONNECTED          = 6,
} bwifi_status_t;

#endif  // BWIFI_PROTO_H
