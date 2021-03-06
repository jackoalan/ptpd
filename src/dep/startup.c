/* startup.c */

#include "../ptpd.h"

PtpClock *ptpClock;

void catch_close(int sig)
{
  char *s;
  
  ptpdShutdown();
  
  switch(sig)
  {
  case SIGINT:
    s = "interrupt";
    break;
    
  case SIGTERM:
    s = "terminate";
    break;
    
  case SIGHUP:
    s = "hangup";
    break;
    
  default:
    s = "?";
  }
  
  NOTIFY("shutdown on %s signal\n", s);
  
  exit(0);
}

void ptpdShutdown()
{
  netShutdown(ptpClock);
  
  free(ptpClock->foreign);
  free(ptpClock);
}

PtpClock * ptpdStartup(int argc, char **argv, Integer16 *ret, RunTimeOpts *rtOpts)
{
  int c, fd = -1, nondaemon = 0, noclose = 0;

  /* parse command line arguments */
  while( (c = getopt(argc, argv, "?cf:dDz:xta:w:b:u:l:o:e:hy:m:gps:i:v:n:k:r")) != -1 ) {
    switch(c) {
    case '?':
      printf(
"\nUsage:  ptpd [OPTION]\n\n"
"-?                show this page\n"
"\n"
"-c                run in command line (non-daemon) mode\n"
"-f FILE           send output to FILE (FILE=syslog writes into the system log)\n"
"-d                display stats\n"
"-D                display stats in .csv format\n"
"\n"
"-z CLOCK          selects which timer is used and controlled\n"
"                  system = the host's system time (default)\n"
"                  nic = the network interface\n"
"                  both = NIC time is synchronized over the network via PTP\n"
"                         and system time against NIC via local PTP\n"
"                  assisted = system time is synchronized across network via\n"
"                             NIC assisted time stamping\n"
"                  linux_hw = synchronize system time with Linux kernel assistance\n"
"                          via net_tstamp API, uses NIC time stamping\n"
"                  linux_sw = synchronize system time with Linux kernel assistance\n"
"                          via net_tstamp API, uses software time stamping\n"
"-x                do not reset the clock if off by more than one second\n"
"-t                do not adjust the system clock\n"
"-a NUMBER,NUMBER  specify clock servo P and I attenuations\n"
"-w NUMBER         specify one way delay filter stiffness\n"
"\n"
"-b NAME           bind PTP to network interface NAME\n"
"-u ADDRESS        also send uni-cast to ADDRESS\n"
"-l NUMBER,NUMBER  specify inbound, outbound latency in nsec\n"
"\n"
"-o NUMBER         specify current UTC offset\n"
"-e NUMBER         specify epoch NUMBER\n"
"-h                specify half epoch\n"
"\n"
"-y NUMBER         specify sync interval in 2^NUMBER sec\n"
"-m NUMBER         specify max number of foreign master records\n"
"\n"
"-g                run as slave only\n"
"-p                make this a preferred clock\n"
"-s NUMBER         specify system clock stratum\n"
"-i NAME           specify system clock identifier\n"
"-v NUMBER         specify system clock allen variance\n"
"\n"
"-n NAME           specify PTP subdomain name (not related to IP or DNS)\n"
"\n"
"-k NUMBER,NUMBER  send a management message of key, record, then exit\n"
"\n"
      );
      *ret = 0;
      return 0;
      
    case 'c':
      nondaemon = 1;
      break;
      
    case 'f':
      if(!strcmp(optarg, "syslog"))
      {
        useSyslog = PTRUE;
      }
      else
      {
        if((fd = creat(optarg, 0400)) != -1)
        {
          dup2(fd, STDOUT_FILENO);
          dup2(fd, STDERR_FILENO);
          noclose = 1;
        }
        else
          PERROR("could not open output file");
      }
      break;
      
    case 'd':
#ifndef PTPD_DBG
      rtOpts->displayStats = PTRUE;
#endif
      break;
      
    case 'D':
      rtOpts->displayStats = PTRUE;
      rtOpts->csvStats = PTRUE;
      break;
      
    case 'z':
      if(!strcasecmp(optarg, "nic"))
      {
        rtOpts->time = TIME_NIC;
      }
      else if(!strcasecmp(optarg, "system"))
      {
        rtOpts->time = TIME_SYSTEM;
      }
      else if(!strcasecmp(optarg, "both"))
      {
        rtOpts->time = TIME_BOTH;
      }
      else if(!strcasecmp(optarg, "assisted"))
      {
        rtOpts->time = TIME_SYSTEM_ASSISTED;
      }
      else if(!strcasecmp(optarg, "linux_hw"))
      {
        rtOpts->time = TIME_SYSTEM_LINUX_HW;
      }
      else if(!strcasecmp(optarg, "linux_sw"))
      {
        rtOpts->time = TIME_SYSTEM_LINUX_SW;
      }
      else
      {
        ERROR("Unsupported -z clock '%s'.\n", optarg);
        *ret = 1;
      }
      break;

    case 'x':
      rtOpts->noResetClock = PTRUE;
      break;
      
    case 't':
      rtOpts->noAdjust = PTRUE;
      break;
      
    case 'a':
      rtOpts->ap = strtol(optarg, &optarg, 0);
      if(optarg[0])
        rtOpts->ai = strtol(optarg+1, 0, 0);
      break;
      
    case 'w':
      rtOpts->s = strtol(optarg, &optarg, 0);
      break;
      
    case 'b':
      memset(rtOpts->ifaceName, 0, IFACE_NAME_LENGTH);
      strncpy(rtOpts->ifaceName, optarg, IFACE_NAME_LENGTH);
      break;
      
    case 'u':
      strncpy(rtOpts->unicastAddress, optarg, NET_ADDRESS_LENGTH);
      break;
      
    case 'l':
      rtOpts->inboundLatency.nanoseconds = strtol(optarg, &optarg, 0);
      if(optarg[0])
        rtOpts->outboundLatency.nanoseconds = strtol(optarg+1, 0, 0);
      break;
      
    case 'o':
      rtOpts->currentUtcOffset = strtol(optarg, &optarg, 0);
      break;
      
    case 'e':
      rtOpts->epochNumber = strtoul(optarg, &optarg, 0);
      break;
      
    case 'h':
      rtOpts->halfEpoch = PTRUE;
      break;
      
    case 'y':
      rtOpts->syncInterval = strtol(optarg, 0, 0);
      break;
      
    case 'm':
      rtOpts->max_foreign_records = strtol(optarg, 0, 0);
      if(rtOpts->max_foreign_records < 1)
        rtOpts->max_foreign_records = 1;
      break;
      
    case 'g':
      rtOpts->slaveOnly = PTRUE;
      break;
      
    case 'p':
      rtOpts->clockPreferred = PTRUE;
      break;
      
    case 's':
      rtOpts->clockStratum = strtol(optarg, 0, 0);
      if(rtOpts->clockStratum <= 0)
        rtOpts->clockStratum = 255;
      break;
      
    case 'i':
      memset(rtOpts->clockIdentifier, 0, PTP_CODE_STRING_LENGTH);
      strncpy(rtOpts->clockIdentifier, optarg, PTP_CODE_STRING_LENGTH);
      break;
      
    case 'v':
      rtOpts->clockVariance = strtol(optarg, 0, 0);
      break;
      
    case 'n':
      memset(rtOpts->subdomainName, 0, PTP_SUBDOMAIN_NAME_LENGTH);
      strncpy(rtOpts->subdomainName, optarg, PTP_SUBDOMAIN_NAME_LENGTH);
      break;
      
    case 'k':
      rtOpts->probe = PTRUE;
      
      rtOpts->probe_management_key = strtol(optarg, &optarg, 0);
      if(optarg[0])
        rtOpts->probe_record_key = strtol(optarg+1, 0, 0);
      
      nondaemon = 1;
      break;
      
    case 'r':
      ERROR("The '-r' option has been removed because it is now the default behaviour.\n");
      ERROR("Use the '-x' option to disable clock resetting.\n");
      *ret = 1;
      return 0;
      
    default:
      *ret = 1;
      return 0;
    }
  }
  
  ptpClock = (PtpClock*)calloc(1, sizeof(PtpClock));
  ptpClock->name = "";
  if(!ptpClock)
  {
    PERROR("failed to allocate memory for protocol engine data");
    *ret = 2;
    return 0;
  }
  else
  {
    ptpClock->runTimeOpts = *rtOpts;
    DBG("allocated %d bytes for protocol engine data\n", (int)sizeof(PtpClock));
    ptpClock->foreign = (ForeignMasterRecord*)calloc(rtOpts->max_foreign_records, sizeof(ForeignMasterRecord));
    if(!ptpClock->foreign)
    {
      PERROR("failed to allocate memory for foreign master data");
      *ret = 2;
      free(ptpClock);
      return 0;
    }
    else
    {
      DBG("allocated %d bytes for foreign master data\n", (int)(rtOpts->max_foreign_records*sizeof(ForeignMasterRecord)));
    }
  }
  
#ifndef PTPD_NO_DAEMON
  if(!nondaemon)
  {
    if(daemon(0, noclose) == -1)
    {
      PERROR("failed to start as daemon");
      *ret = 3;
      return 0;
    }
    DBG("running as daemon\n");
  }
#endif
  
  signal(SIGINT, catch_close);
  signal(SIGTERM, catch_close);
  signal(SIGHUP, catch_close);
  
  *ret = 0;
  return ptpClock;
}

