#include <AccelStepper.h>


// Time in milliseconds
#define AXIS_UPDATE_INTERVAL      100
#define SPEED_UPDATE_INTERVAL     200
#define LIMIT_CHECK_INTERVAL      200

#define SPEED_MIN                 200
#define SPEED_MAX                 3000
// Time in seconds to wait before reversing direction again.
#define DIRECTION_CHANGE_TIME     10


#define AXIS_0_PIN_STEP             11
#define AXIS_0_PIN_DIR              12
#define AXIS_0_PIN_ENABLE           13
AccelStepper motor0(AccelStepper::DRIVER, AXIS_0_PIN_STEP, AXIS_0_PIN_DIR);


#define SPEED_CONTROL               A0
// Normal closed limit switch. (internal pullup is enabled)
#define LIMIT_SW                    8
#define LIMIT_LED                   9


typedef struct _AxisState {
  AccelStepper  *motor;
  unsigned long last_update_time;
  float         acceleration;
  float         current_speed;
  float         target_speed;
  bool          is_active;
  bool          direction_cw;
} AxisState;

unsigned long last_speed_update_time = 0;
unsigned long last_limit_check_time = 0;
unsigned int acceleration    = 5000;

/* forward declarations */
static inline void custom_loop();
void set_axis_active (AxisState *axis, bool active);
static inline void compute_axis_speed(AxisState *axis);
void update_axis_speeds(AxisState *axis);

AxisState axis0;

static inline
void compute_axis_speed(AxisState *axis)
{
  float current_speed, target_speed, new_speed;
  unsigned long now;
  signed long   interval;

  if (!axis)  {
    return;
  }

  now = millis();

  interval = now - axis->last_update_time;
  interval = interval > 0 ? interval : -interval;
  if (interval < AXIS_UPDATE_INTERVAL) {
    axis->motor->runSpeed();
    return;
  }

  axis->last_update_time = now;

  current_speed = axis->current_speed;
  target_speed  = axis->direction_cw ? axis->target_speed : -1.0f * axis->target_speed;

  if (current_speed != target_speed) {
    if (current_speed < target_speed) {
      new_speed = current_speed + axis->acceleration;
      if (new_speed > target_speed) {
        new_speed = target_speed;
      }
    } else {
      new_speed = current_speed - axis->acceleration;
      if (new_speed < target_speed) {
        new_speed = target_speed;
      }
    }
    axis->current_speed = new_speed;
  }

  axis->motor->setSpeed(axis->current_speed);
  axis->motor->runSpeed();
}

void
set_axis_accelleration(AxisState *axis, int acceleration)
{
  axis->acceleration = acceleration * 0.001 * AXIS_UPDATE_INTERVAL;
}

void
set_axis_active(AxisState *axis, bool active)
{
  axis->is_active = active;
  if (!active) {
    axis->target_speed = 0;
  }
}


void setup()
{
  pinMode(AXIS_0_PIN_STEP,   OUTPUT);
  pinMode(AXIS_0_PIN_DIR,    OUTPUT);
  pinMode(AXIS_0_PIN_ENABLE, OUTPUT);

  pinMode(LIMIT_SW, INPUT);
  digitalWrite(LIMIT_SW, 1);
  pinMode(LIMIT_LED, OUTPUT);

  analogReference(DEFAULT);

  motor0.setEnablePin(AXIS_0_PIN_ENABLE);
  motor0.setMinPulseWidth(500);
  motor0.setMaxSpeed(SPEED_MAX);
  motor0.setAcceleration(acceleration);
  motor0.setCurrentPosition(0);
  motor0.enableOutputs();

  axis0.motor         = &motor0;
  axis0.current_speed = 0;
  axis0.target_speed  = 400;
  axis0.direction_cw  = false;
  set_axis_accelleration(&axis0, acceleration);
  set_axis_active(&axis0, true);
}

void loop()
{
  compute_axis_speed(&axis0);

  unsigned long now = millis();
  if (now < last_speed_update_time) {
    // Wraparound.
    last_speed_update_time = 0;
  }

  if ((now - last_speed_update_time) > SPEED_UPDATE_INTERVAL) {
    last_speed_update_time = now;
    int pot = analogRead(SPEED_CONTROL);
    float target_speed = map(pot, 0, 1023, SPEED_MIN, SPEED_MAX);
    axis0.target_speed = target_speed;
  }

  bool limit_sw = digitalRead(LIMIT_SW);
  digitalWrite(LIMIT_LED, limit_sw);
  if ((now > last_limit_check_time) && ((now - last_limit_check_time) > LIMIT_CHECK_INTERVAL)) {
    last_limit_check_time = now;
    // Active HIGH with pull-up
    if (limit_sw) {
      // delay for a while until checking again, give time to reverse motion.
      last_limit_check_time = now + 1000*DIRECTION_CHANGE_TIME;
      axis0.direction_cw = !axis0.direction_cw;
    }
  }
}
