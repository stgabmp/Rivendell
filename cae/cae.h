// cae.h
//
// The Core Audio Engine component of Rivendell
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: cae.h,v 1.70 2008/03/28 20:00:05 fredg Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//


#ifndef CAE_H
#define CAE_H

#include <sys/types.h>
#include <pthread.h>

#include <qobject.h>
#include <qstring.h>
#include <qserversocket.h>
#include <qsignalmapper.h>
#include <qtimer.h>
#include <rdwavefile.h>
#include <rdsocket.h>

#ifdef HPI
#include <rdhpisoundcard.h>
#include <rdhpiplaystream.h>
#include <rdhpirecordstream.h>
#endif  // HPI

#ifdef ALSA
#include <alsa/asoundlib.h>
struct alsa_format {
  int card;
  pthread_t thread;
  snd_pcm_t *pcm;
  unsigned channels;
  unsigned capture_channels;
  snd_pcm_uframes_t buffer_size;
  snd_pcm_format_t format;
  unsigned sample_rate;
  char *card_buffer;
  char *passthrough_buffer;
  unsigned card_buffer_size;
  unsigned periods;
  bool exiting;
};
#endif  // ALSA

#ifdef JACK
#include <jack/jack.h>
#endif  // JACK

#include <rd.h>
#include <rdconfig.h>
#include <rdstation.h>

//
// Debug Options
//
//#define PRINT_COMMANDS

//
// Global CAE Definitions
//
#define RINGBUFFER_SIZE 131072
#define CAED_USAGE "[-d]\n\nSupplying the '-d' flag will set 'debug' mode, causing caed(8) to stay\nin the foreground and print debugging info on standard output.\n" 

//
// Function Prototypes
//
void LogLine(RDConfig::LogPriority prio,const QString &line);
void SigHandler(int signum);
extern RDConfig *rd_config;


class MainObject : public QObject
{
  Q_OBJECT
 public:
  MainObject(QObject *parent=0,const char *name=0);
  ~MainObject();

 public slots:
  void newConnection(int fd);

 private slots:
  void socketData(int);
  void socketKill(int);
  void statePlayUpdate(int card,int stream,int state);
  void stateRecordUpdate(int card,int stream,int state);
  void updateMeters();
  
 private:
  void InitMixers();
  void ParseCommand(int);
  void DispatchCommand(int);
  void KillSocket(int);
  void BroadcastCommand(const char *);
  void EchoCommand(int,const char *);
  void EchoArgs(int,const char);
  bool CheckDaemon(QString);
  pid_t GetPid(QString pidfile);
  int GetNextHandle();
  int GetHandle(int ch,int *card,int *stream);
  int GetHandle(int card,int stream);
  long Resample(short *data_in,short *data_out,long frames,
		    double ratio,int chans);
  long Resample(float *data_in,float *data_out,long frames,
		    double ratio,int chans);
  void ProbeCaps(RDStation *station);
  void ClearDriverEntries(RDStation *station);
  bool debug;
  Q_INT16 tcp_port;
  QServerSocket *server;
  RDSocket *socket[CAE_MAX_CONNECTIONS];
  char args[CAE_MAX_CONNECTIONS][CAE_MAX_ARGS][CAE_MAX_LENGTH];
  int istate[CAE_MAX_CONNECTIONS];
  int argnum[CAE_MAX_CONNECTIONS];
  int argptr[CAE_MAX_CONNECTIONS];
  bool auth[CAE_MAX_CONNECTIONS];
  RDStation::AudioDriver cae_driver[RD_MAX_CARDS];
  int record_owner[RD_MAX_CARDS][RD_MAX_STREAMS];
  int record_length[RD_MAX_CARDS][RD_MAX_STREAMS];
  int record_threshold[RD_MAX_CARDS][RD_MAX_STREAMS];
  int play_owner[RD_MAX_CARDS][RD_MAX_STREAMS];
  int play_length[RD_MAX_CARDS][RD_MAX_STREAMS];
  int play_speed[RD_MAX_CARDS][RD_MAX_STREAMS];
  bool play_pitch[RD_MAX_CARDS][RD_MAX_STREAMS];
  bool port_status[RD_MAX_CARDS][RD_MAX_PORTS];
  struct {
    int card;
    int stream;
    int owner;
  } play_handle[256];
  int next_play_handle;
  float *resample_buffer[2];
  float *short_resample_buffer[2];

  //
  // HPI Driver
  //
 private:
  void hpiInit(RDStation *station);
  void hpiFree();
  bool hpiLoadPlayback(int card,QString wavename,int *stream);
  bool hpiUnloadPlayback(int card,int stream);
  bool hpiPlaybackPosition(int card,int stream,unsigned pos);
  bool hpiPlay(int card,int stream,int length,int speed,bool pitch,
	       bool rates);
  bool hpiStopPlayback(int card,int stream);
  bool hpiTimescaleSupported(int card);
  bool hpiLoadRecord(int card,int port,int coding,int chans,int samprate,
		     int bitrate,QString wavename);
  bool hpiUnloadRecord(int card,int stream);
  bool hpiRecord(int card,int stream,int length,int thres);
  bool hpiStopRecord(int card,int stream);
  bool hpiSetInputVolume(int card,int stream,int level);
  bool hpiSetOutputVolume(int card,int stream,int port,int level);
  bool hpiFadeOutputVolume(int card,int stream,int port,int level,int length);
  bool hpiSetInputLevel(int card,int port,int level);
  bool hpiSetOutputLevel(int card,int port,int level);
  bool hpiSetInputMode(int card,int stream,int mode);
  bool hpiSetOutputMode(int card,int stream,int mode);
  bool hpiSetInputVoxLevel(int card,int stream,int level);
  bool hpiSetInputType(int card,int port,int type);
  bool hpiGetInputStatus(int card,int port);
  bool hpiGetInputMeters(int card,int port,short levels[2]);
  bool hpiGetOutputMeters(int card,int port,short levels[2]);
  bool hpiSetPassthroughLevel(int card,int in_port,int out_port,int level);
  void hpiGetOutputPosition(int card,unsigned *pos);
#ifdef HPI
  RDHPISoundCard *sound_card;
  RDHPIRecordStream *record[RD_MAX_CARDS][RD_MAX_STREAMS];
  RDHPIPlayStream *play[RD_MAX_CARDS][RD_MAX_STREAMS];
#endif  // HPI

  //
  // JACK Driver
  //
 private slots:
  void jackStopTimerData(int stream);
  void jackFadeTimerData(int stream);
  void jackRecordTimerData(int stream);

 private:
  void jackInit(RDStation *station);
  void jackFree();
  bool jackLoadPlayback(int card,QString wavename,int *stream);
  bool jackUnloadPlayback(int card,int stream);
  bool jackPlaybackPosition(int card,int stream,unsigned pos);
  bool jackPlay(int card,int stream,int length,int speed,bool pitch,
	       bool rates);
  bool jackStopPlayback(int card,int stream);
  bool jackTimescaleSupported(int card);
  bool jackLoadRecord(int card,int port,int coding,int chans,int samprate,
		     int bitrate,QString wavename);
  bool jackUnloadRecord(int card,int stream);
  bool jackRecord(int card,int stream,int length,int thres);
  bool jackStopRecord(int card,int stream);
  bool jackSetInputVolume(int card,int stream,int level);
  bool jackSetOutputVolume(int card,int stream,int port,int level);
  bool jackFadeOutputVolume(int card,int stream,int port,int level,int length);
  bool jackSetInputLevel(int card,int port,int level);
  bool jackSetOutputLevel(int card,int port,int level);
  bool jackSetInputMode(int card,int stream,int mode);
  bool jackSetOutputMode(int card,int stream,int mode);
  bool jackSetInputVoxLevel(int card,int stream,int level);
  bool jackSetInputType(int card,int port,int type);
  bool jackGetInputStatus(int card,int port);
  bool jackGetInputMeters(int card,int port,short levels[2]);
  bool jackGetOutputMeters(int card,int port,short levels[2]);
  bool jackSetPassthroughLevel(int card,int in_port,int out_port,int level);
  void jackGetOutputPosition(int card,unsigned *pos);
  int GetJackOutputStream();
  void FreeJackOutputStream(int stream);
  void EmptyJackInputStream(int stream);
  void FillJackOutputStream(int stream);
  void JackClock();
  void JackSessionSetup();
  bool jack_connected;
  bool jack_activated;
#ifdef JACK
  int jack_card;
  RDWaveFile *jack_record_wave[RD_MAX_STREAMS];
  RDWaveFile *jack_play_wave[RD_MAX_STREAMS];
  short *jack_wave_buffer;
  jack_default_audio_sample_t *jack_sample_buffer;
  jack_default_audio_sample_t *jack_resample_buffer;
  short jack_input_volume_db[RD_MAX_STREAMS];
  short jack_output_volume_db[RD_MAX_PORTS][RD_MAX_STREAMS];
  short jack_passthrough_volume_db[RD_MAX_PORTS][RD_MAX_PORTS];
  short jack_fade_volume_db[RD_MAX_STREAMS];
  short jack_fade_increment[RD_MAX_STREAMS];
  int jack_fade_port[RD_MAX_STREAMS];
  bool jack_fade_up[RD_MAX_STREAMS];
  QTimer *jack_fade_timer[RD_MAX_STREAMS];
  QTimer *jack_stop_timer[RD_MAX_STREAMS];
  QTimer *jack_record_timer[RD_MAX_PORTS];
  int jack_offset[RD_MAX_STREAMS];
  int jack_clock_phase;
#endif  // JACK

  //
  // ALSA Driver
  //
 private slots:
  void alsaStopTimerData(int cardstream);
  void alsaFadeTimerData(int cardstream);
  void alsaRecordTimerData(int cardport);

 private:
  void alsaInit(RDStation *station);
  void alsaFree();
  bool alsaLoadPlayback(int card,QString wavename,int *stream);
  bool alsaUnloadPlayback(int card,int stream);
  bool alsaPlaybackPosition(int card,int stream,unsigned pos);
  bool alsaPlay(int card,int stream,int length,int speed,bool pitch,
	       bool rates);
  bool alsaStopPlayback(int card,int stream);
  bool alsaTimescaleSupported(int card);
  bool alsaLoadRecord(int card,int port,int coding,int chans,int samprate,
		     int bitrate,QString wavename);
  bool alsaUnloadRecord(int card,int stream);
  bool alsaRecord(int card,int stream,int length,int thres);
  bool alsaStopRecord(int card,int stream);
  bool alsaSetInputVolume(int card,int stream,int level);
  bool alsaSetOutputVolume(int card,int stream,int port,int level);
  bool alsaFadeOutputVolume(int card,int stream,int port,int level,int length);
  bool alsaSetInputLevel(int card,int port,int level);
  bool alsaSetOutputLevel(int card,int port,int level);
  bool alsaSetInputMode(int card,int stream,int mode);
  bool alsaSetOutputMode(int card,int stream,int mode);
  bool alsaSetInputVoxLevel(int card,int stream,int level);
  bool alsaSetInputType(int card,int port,int type);
  bool alsaGetInputStatus(int card,int port);
  bool alsaGetInputMeters(int card,int port,short levels[2]);
  bool alsaGetOutputMeters(int card,int port,short levels[2]);
  bool alsaSetPassthroughLevel(int card,int in_port,int out_port,int level);
  void alsaGetOutputPosition(int card,unsigned *pos);
  void AlsaClock();
#ifdef ALSA
  bool AlsaStartCaptureDevice(QString &dev,int card,snd_pcm_t *pcm);
  bool AlsaStartPlayDevice(QString &dev,int card,snd_pcm_t *pcm);
  void AlsaInitCallback();
  int GetAlsaOutputStream(int card);
  void FreeAlsaOutputStream(int card,int stream);
  void EmptyAlsaInputStream(int card,int stream);
  void FillAlsaOutputStream(int card,int stream);
  struct alsa_format alsa_play_format[RD_MAX_CARDS];
  struct alsa_format alsa_capture_format[RD_MAX_CARDS];
  short alsa_input_volume_db[RD_MAX_CARDS][RD_MAX_STREAMS];
  short alsa_output_volume_db[RD_MAX_CARDS][RD_MAX_PORTS][RD_MAX_STREAMS];
  short alsa_passthrough_volume_db[RD_MAX_CARDS][RD_MAX_PORTS][RD_MAX_PORTS];
  short *alsa_wave_buffer;
  short *alsa_resample_buffer;
  RDWaveFile *alsa_record_wave[RD_MAX_CARDS][RD_MAX_STREAMS];
  RDWaveFile *alsa_play_wave[RD_MAX_CARDS][RD_MAX_STREAMS];
  int alsa_offset[RD_MAX_CARDS][RD_MAX_STREAMS];
  QTimer *alsa_fade_timer[RD_MAX_CARDS][RD_MAX_STREAMS];
  QTimer *alsa_stop_timer[RD_MAX_CARDS][RD_MAX_STREAMS];
  QTimer *alsa_record_timer[RD_MAX_CARDS][RD_MAX_PORTS];
  bool alsa_fade_up[RD_MAX_CARDS][RD_MAX_STREAMS];
  short alsa_fade_volume_db[RD_MAX_CARDS][RD_MAX_STREAMS];
  short alsa_fade_increment[RD_MAX_CARDS][RD_MAX_STREAMS];
  int alsa_fade_port[RD_MAX_CARDS][RD_MAX_STREAMS];
#endif  // ALSA
};


#endif  // CAE_H
