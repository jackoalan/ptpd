/* datatypes.h */

#ifndef DATATYPES_H
#define DATATYPES_H

typedef struct {
  UInteger32 seconds;
  Integer32 nanoseconds;  
} TimeRepresentation;

typedef struct {
  Integer32 seconds;
  Integer32 nanoseconds;  
} TimeInternal;

typedef struct {
  Integer32  interval;
  Integer32  left;
  ptpdBoolean expire;
} IntervalTimer;

/* Message header */
typedef struct {
  UInteger16  versionPTP;
  UInteger16  versionNetwork;
  Octet subdomain[PTP_SUBDOMAIN_NAME_LENGTH];
  UInteger8  messageType;
  UInteger8  sourceCommunicationTechnology;
  Octet  sourceUuid[PTP_UUID_LENGTH];
  UInteger16  sourcePortId;
  UInteger16  sequenceId;
  UInteger8  control;
  Octet  flags[2];
  
} MsgHeader;

/* Sync or Delay_Req message */
typedef struct {
  TimeRepresentation  originTimestamp;
  UInteger16  epochNumber;
  Integer16  currentUTCOffset;
  UInteger8  grandmasterCommunicationTechnology;
  Octet  grandmasterClockUuid[PTP_UUID_LENGTH];
  UInteger16  grandmasterPortId;
  UInteger16  grandmasterSequenceId;
  UInteger8  grandmasterClockStratum;
  Octet  grandmasterClockIdentifier[PTP_CODE_STRING_LENGTH];
  Integer16  grandmasterClockVariance;
  ptpdBoolean  grandmasterPreferred;
  ptpdBoolean  grandmasterIsBoundaryClock;
  Integer8  syncInterval;
  Integer16  localClockVariance;
  UInteger16  localStepsRemoved;
  UInteger8  localClockStratum;
  Octet  localClockIdentifer[PTP_CODE_STRING_LENGTH];
  UInteger8  parentCommunicationTechnology;
  Octet  parentUuid[PTP_UUID_LENGTH];
  UInteger16  parentPortField;
  Integer16  estimatedMasterVariance;
  Integer32  estimatedMasterDrift;
  ptpdBoolean  utcReasonable;
  
} MsgSync;

typedef MsgSync MsgDelayReq;

/* Follow_Up message */
typedef struct {
  UInteger16  associatedSequenceId;
  TimeRepresentation  preciseOriginTimestamp;
  
} MsgFollowUp;

/* Delay_Resp message */
typedef struct {
  TimeRepresentation  delayReceiptTimestamp;
  UInteger8  requestingSourceCommunicationTechnology;
  Octet  requestingSourceUuid[PTP_UUID_LENGTH];
  UInteger16  requestingSourcePortId;
  UInteger16  requestingSourceSequenceId;
  
} MsgDelayResp;

/* Management message */
typedef union
{
  struct ClockIdentity
  {
    UInteger8  clockCommunicationTechnology;
    Octet  clockUuidField[PTP_UUID_LENGTH];
    UInteger16  clockPortField;
    Octet  manufacturerIdentity[MANUFACTURER_ID_LENGTH];
  } clockIdentity;
  
  struct DefaultData
  {
    UInteger8  clockCommunicationTechnology;
    Octet  clockUuidField[PTP_UUID_LENGTH];
    UInteger16  clockPortField;
    UInteger8  clockStratum;
    Octet  clockIdentifier[PTP_CODE_STRING_LENGTH];
    Integer16  clockVariance;
    ptpdBoolean  clockFollowupCapable;
    ptpdBoolean  preferred;
    ptpdBoolean  initializable;
    ptpdBoolean  externalTiming;
    ptpdBoolean  isBoundaryClock;
    Integer8  syncInterval;
    Octet  subdomainName[PTP_SUBDOMAIN_NAME_LENGTH];
    UInteger16  numberPorts;
    UInteger16  numberForeignRecords;
  } defaultData;
  
  struct Current
  {
    UInteger16  stepsRemoved;
    TimeRepresentation  offsetFromMaster;
    TimeRepresentation  oneWayDelay;
  } current;
  
  struct Parent
  {
    UInteger8  parentCommunicationTechnology;
    Octet  parentUuid[PTP_UUID_LENGTH];
    UInteger16  parentPortId;
    UInteger16  parentLastSyncSequenceNumber;
    ptpdBoolean  parentFollowupCapable;
    ptpdBoolean  parentExternalTiming;
    Integer16  parentVariance;
    ptpdBoolean  parentStats;
    Integer16  observedVariance;
    Integer32  observedDrift;
    ptpdBoolean  utcReasonable;
    UInteger8  grandmasterCommunicationTechnology;
    Octet  grandmasterUuidField[PTP_UUID_LENGTH];
    UInteger16  grandmasterPortIdField;
    UInteger8  grandmasterStratum;
    Octet  grandmasterIdentifier[PTP_CODE_STRING_LENGTH];
    Integer16  grandmasterVariance;
    ptpdBoolean  grandmasterPreferred;
    ptpdBoolean  grandmasterIsBoundaryClock;
    UInteger16  grandmasterSequenceNumber;
  } parent;
  
  struct Port
  {
    UInteger16  returnedPortNumber;
    UInteger8  portState;
    UInteger16  lastSyncEventSequenceNumber;
    UInteger16  lastGeneralEventSequenceNumber;
    UInteger8  portCommunicationTechnology;
    Octet  portUuidField[PTP_UUID_LENGTH];
    UInteger16  portIdField;
    ptpdBoolean  burstEnabled;
    UInteger8  subdomainAddressOctets;
    UInteger8  eventPortAddressOctets;
    UInteger8  generalPortAddressOctets;
    Octet  subdomainAddress[SUBDOMAIN_ADDRESS_LENGTH];
    Octet  eventPortAddress[PORT_ADDRESS_LENGTH];
    Octet  generalPortAddress[PORT_ADDRESS_LENGTH];
  } port;
  
  struct GlobalTime
  {
    TimeRepresentation  localTime;
    Integer16  currentUtcOffset;
    ptpdBoolean  leap59;
    ptpdBoolean  leap61;
    UInteger16  epochNumber;
  } globalTime;
  
  struct Foreign
  {
    UInteger16  returnedPortNumber;
    UInteger16  returnedRecordNumber;
    UInteger8  foreignMasterCommunicationTechnology;
    Octet  foreignMasterUuid[PTP_UUID_LENGTH];
    UInteger16  foreignMasterPortId;
    UInteger16  foreignMasterSyncs;
  } foreign;
  
} MsgManagementPayload;

typedef struct {
  UInteger8  targetCommunicationTechnology;
  Octet  targetUuid[PTP_UUID_LENGTH];
  UInteger16  targetPortId;
  Integer16  startingBoundaryHops;
  Integer16  boundaryHops;
  UInteger8  managementMessageKey;
  UInteger16  parameterLength;
  UInteger16  recordKey;
  
  MsgManagementPayload payload;
} MsgManagement;

typedef struct
{
  UInteger8  foreign_master_communication_technology;
  Octet  foreign_master_uuid[PTP_UUID_LENGTH];
  UInteger16  foreign_master_port_id;
  UInteger16  foreign_master_syncs;
  
  MsgHeader  header;
  MsgSync  sync;
} ForeignMasterRecord;

typedef enum {
  TIME_SYSTEM,     /**< use and control system time */
  TIME_NIC,        /**< use and control time inside network interface (via Intel
                      igb ioctl()) */
  /**
   * a combination of PTP between NICs (as in TIME_NIC via Intel igb
   * ioctl()) plus a local synchronization between NIC and system
   * time:
   *
   * - NIC time is controlled via PTP packets, main time seen
   *   by PTPd is the NIC time
   * - system_to_nic and nic_to_system delays are provided by
   *   device driver on request
   * - time.c adapts NIC time to system time on master and system time
   *   to NIC time on slaves by feeding these offsets into another
   *   instance of the clock servo (as in TIME_SYSTEM)
   *
   * The command line options only apply to one clock sync and
   * the defaults are used for the other:
   * - NIC time: default values for clock control (adjust and reset)
   *   and servo (coefficients), configurable PTP
   * - system time: configurable clock control and servo, PTP options do
   *   not apply
   */
  TIME_BOTH,
  /**
   * time used and controlled by PTP is the system time, but hardware
   * assistance in the NIC is used to time stamp packages (via Intel
   * igb ioctl())
   */
  TIME_SYSTEM_ASSISTED,
  /**
   * Time used and controlled by PTP is the system time with hardware
   * packet time stamping via standard Linux net_tstamp.h API.
   */
  TIME_SYSTEM_LINUX_HW,
  /**
   * Time used and controlled by PTP is the system time with software
   * packet time stamping via standard Linux net_tstamp.h API.
   */
  TIME_SYSTEM_LINUX_SW,

  TIME_MAX
} Time;

/* program options set at run-time */
typedef struct {
  Integer8  syncInterval;
  Octet  subdomainName[PTP_SUBDOMAIN_NAME_LENGTH];
  Octet  clockIdentifier[PTP_CODE_STRING_LENGTH];
  UInteger32  clockVariance;
  UInteger8  clockStratum;
  ptpdBoolean  clockPreferred;
  Integer16  currentUtcOffset;
  UInteger16  epochNumber;
  Octet  ifaceName[IFACE_NAME_LENGTH];
  ptpdBoolean  noResetClock;
  ptpdBoolean  noAdjust;
  ptpdBoolean  displayStats;
  ptpdBoolean  csvStats;
  Octet  unicastAddress[NET_ADDRESS_LENGTH];
  Integer16  ap, ai;
  Integer16  s;
  TimeInternal  inboundLatency, outboundLatency;
  Integer16  max_foreign_records;
  ptpdBoolean  slaveOnly;
  ptpdBoolean  probe;
  UInteger8  probe_management_key;
  UInteger16  probe_record_key;
  ptpdBoolean  halfEpoch;
  Time time;
} RunTimeOpts;

/* main program data structure */
typedef struct {
  /* settings associate with this instance of PtpClock */
  RunTimeOpts runTimeOpts;

  /* Default data set */
  UInteger8  clock_communication_technology;
  Octet  clock_uuid_field[PTP_UUID_LENGTH];
  UInteger16  clock_port_id_field;
  UInteger8  clock_stratum;
  Octet  clock_identifier[PTP_CODE_STRING_LENGTH];
  Integer16  clock_variance;
  ptpdBoolean  clock_followup_capable;
  ptpdBoolean  preferred;
  ptpdBoolean  initializable;
  ptpdBoolean  external_timing;
  ptpdBoolean  is_boundary_clock;
  Integer8  sync_interval;
  Octet  subdomain_name[PTP_SUBDOMAIN_NAME_LENGTH];
  UInteger16  number_ports;
  UInteger16  number_foreign_records;
  
  /* Current data set */
  UInteger16  steps_removed;
  TimeInternal  offset_from_master;
  TimeInternal  one_way_delay;
  
  /* Parent data set */
  UInteger8  parent_communication_technology;
  Octet  parent_uuid[PTP_UUID_LENGTH];
  UInteger16  parent_port_id;
  UInteger16  parent_last_sync_sequence_number;
  ptpdBoolean  parent_followup_capable;
  ptpdBoolean  parent_external_timing;
  Integer16  parent_variance;
  ptpdBoolean  parent_stats;
  Integer16  observed_variance;
  Integer32  observed_drift;
  long       adj;
  ptpdBoolean  utc_reasonable;
  UInteger8  grandmaster_communication_technology;
  Octet  grandmaster_uuid_field[PTP_UUID_LENGTH];
  UInteger16  grandmaster_port_id_field;
  UInteger8  grandmaster_stratum;
  Octet  grandmaster_identifier[PTP_CODE_STRING_LENGTH];
  Integer16  grandmaster_variance;
  ptpdBoolean  grandmaster_preferred;
  ptpdBoolean  grandmaster_is_boundary_clock;
  UInteger16  grandmaster_sequence_number;
  
  /* Global time properties data set */
  Integer16  current_utc_offset;
  ptpdBoolean  leap_59;
  ptpdBoolean  leap_61;
  UInteger16  epoch_number;
  
  /* Port configuration data set */
  UInteger8  port_state;
  UInteger16  last_sync_event_sequence_number;
  UInteger16  last_general_event_sequence_number;
  Octet  subdomain_address[SUBDOMAIN_ADDRESS_LENGTH];
  Octet  event_port_address[PORT_ADDRESS_LENGTH];
  Octet  general_port_address[PORT_ADDRESS_LENGTH];
  UInteger8  port_communication_technology;
  Octet  port_uuid_field[PTP_UUID_LENGTH];
  UInteger16  port_id_field;
  ptpdBoolean  burst_enabled;
  
  /* Foreign master data set */
  ForeignMasterRecord *foreign;
  
  /* Other things we need for the protocol */
  ptpdBoolean halfEpoch;
  
  Integer16  max_foreign_records;
  Integer16  foreign_record_i;
  Integer16  foreign_record_best;
  ptpdBoolean  record_update;
  UInteger32 random_seed;
  
  MsgHeader msgTmpHeader;
  
  union {
    MsgSync  sync;
    MsgFollowUp  follow;
    MsgDelayReq  req;
    MsgDelayResp  resp;
    MsgManagement  manage;
  } msgTmp;
  
  Octet msgObuf[PACKET_SIZE];
  Octet msgIbuf[PACKET_SIZE];
  
  TimeInternal  master_to_slave_delay;
  TimeInternal  slave_to_master_delay;
  
  TimeInternal  delay_req_receive_time;
  TimeInternal  delay_req_send_time;
  TimeInternal  sync_receive_time;
  
  UInteger16  Q;
  UInteger16  R;

  /**
   * TRUE when the clock is used to synchronize NIC and system time and
   * the process is the PTP_MASTER. The offset and adjustment calculation
   * is always "master (= NIC) to slave (= system time)" and PTP_SLAVEs
   * update their system time, but the master needs to invert the
   * clock adjustment and control NIC time instead. This way
   * the master's system time is propagated to slaves.
   */
  ptpdBoolean nic_instead_of_system;

  /**
   * TRUE if outgoing packets are not to be time-stamped in advance.
   * Instead the outgoing time stamp is generated as it is transmitted
   * and must be sent in a follow-up message.
   */
  ptpdBoolean delayedTiming;

  /**
   * a prefix to be inserted before messages about the clock:
   * may be empty, but not NULL
   *
   * used to distinguish multiple active clock servos per process
   */
  const char *name;

  ptpdBoolean  sentDelayReq;
  UInteger16  sentDelayReqSequenceId;
  ptpdBoolean  waitingForFollow;
  
  offset_from_master_filter  ofm_filt;
  one_way_delay_filter  owd_filt;
  
  ptpdBoolean message_activity;
  
  IntervalTimer  itimer[TIMER_ARRAY_SIZE];
  
  NetPath netPath;

} PtpClock;

#endif
