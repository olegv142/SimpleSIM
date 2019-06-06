//
// The example SMS echo server based on SIM800L GSM module.
// Send any message starting with # and
// you will receive it back with signal quality report appended.
//
// The module is connected to the hardware serial port.
// The reset pin of the module is connected to pin 2 of Arduino.
// Note that the GSM module has special power requirements:
// 1) 3.5-4.5V is recommended. So 3.3V is too small while 5V is too high
// 2) The peak consumption may be up to 2A while the idle current is much lower.
//    So either use powerful enough adaptor or LiPo cell or super-capacitor
//    with low internal resistance (<= 0.2 Ohm).
// 3) The internal logic of GSM module has power voltage level around 2.8V.
//    So the simplest approach to interfacing it is to use 3.3V for Arduino
//    and connect its TX line to the module RX pin via 1k + 5.6k resistive divider.
//
// You may use software serial port instead of the hardware one but keep in mind
// that it makes your system inherently unreliable since it used to disable interrupts
// for the entire byte transmission duration.
//

#include <SimpleSIM.h>

#define GSM_BAUDS 9600
#define GSM_MAX_ERRS 10
#define GSM_RST_PIN  2

SimpleSIM          g_gsm(Serial, GSM_RST_PIN);
static bool        g_gsm_started;
static int         g_gsm_err_cnt;
static SIMHook     g_gsm_csq("+CSQ");
static SIMHook     g_gsm_cmt("+CMT");
static SIMHook     g_gsm_msg("#");

static void init_gsm()
{
  Serial.begin(GSM_BAUDS);
  g_gsm.add_hook(&g_gsm_csq);
  g_gsm.add_hook(&g_gsm_cmt);
  g_gsm.add_hook(&g_gsm_msg);
  g_gsm.begin();
}

static void reset_gsm()
{
  g_gsm.reset();
  g_gsm_err_cnt = 0;
  g_gsm_started = false;
}

static void on_gsm_err()
{
  if (++g_gsm_err_cnt >= GSM_MAX_ERRS) {
    reset_gsm();
  }
}

static void process_gsm()
{
  if (!g_gsm_started) {
    if (g_gsm.start(GSM_BAUDS) == sim_ok) {
      g_gsm_started = true;
    } else {
      on_gsm_err();
    }
  } else {
    if (g_gsm_msg) {
      String sms_cmd("+CMGS=");
      sms_cmd += g_gsm_cmt.str().substring(6, 20);
      if (
        sim_ok     == g_gsm.send_cmd("+CSQ") &&
        sim_prompt == g_gsm.send_cmd(sms_cmd.c_str()) &&
        sim_ok     == g_gsm.send_msg((g_gsm_msg.str() + g_gsm_csq.str()).c_str())
      ) {
        g_gsm_msg.reset();
      } else {
        on_gsm_err();
      }
    }
  }
}

void setup() {
  init_gsm();
}

void loop() {
  g_gsm.wait(1000);
  process_gsm();
}
