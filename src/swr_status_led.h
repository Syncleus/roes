#ifndef _SWR_STATUS_LED_H_
#define _SWR_STATUS_LED_H_

enum LedStatus {
  OFF,
  ON,
  SLOW,
  FAST,
  VERY_FAST
};

void statusLedSetup();
void statusLedUpdate();
void setLedStatus(LedStatus newStatus);

#endif /* _SWR_STATUS_LED_H_ */
