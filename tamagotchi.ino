#include <SPI.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  0xFC20
#define BROWN   0x92C4
#define LBLUE   0x7E7B
#define __CS 10
#define __DC 9

TFT_ILI9163C display = TFT_ILI9163C(__CS, 8, __DC);

typedef struct task {
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);
} task;

const unsigned short numTasks = 12;
task tasks[numTasks];

unsigned char pose1[14][14] =
{ {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
  {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
  {1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
};

unsigned char pose2[14][14] =
{ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0},
  {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
  {1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0},
  {0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
};

int birth_time = millis();

unsigned char x_pos = 50;
unsigned char y_pos = 50;
unsigned char poseNum = 1;

unsigned char pixel_size = 3;
unsigned char char_pixel_size = 14;
unsigned char char_size = pixel_size * char_pixel_size;

float health = 15.0;
float energy = 30.0;
float hunger = 20.0;
float happiness = 30.0;

float hunger_decay_amount = 1;
float energy_decay_amount = 1;

String curr_scene = "main";
bool move_enabled = false;

unsigned char lightning_icon[8][8] =
{ {0, 0, 0, 1, 1, 1, 1, 0},
  {0, 0, 1, 2, 2, 2, 2, 1},
  {0, 1, 2, 2, 2, 2, 1, 0},
  {1, 2, 2, 2, 2, 1, 1, 0},
  {0, 1, 1, 1, 2, 2, 2, 1},
  {0, 0, 0, 0, 1, 2, 1, 0},
  {0, 0, 0, 1, 2, 1, 0, 0},
  {0, 0, 0, 0, 1, 0, 0, 0},
};

unsigned char hunger_icon[8][8] =
{ {0, 0, 0, 0, 1, 1, 1, 0},
  {0, 0, 0, 1, 2, 0, 2, 1},
  {0, 0, 1, 2, 2, 2, 2, 1},
  {0, 0, 1, 2, 2, 2, 2, 1},
  {0, 0, 1, 2, 2, 2, 1, 0},
  {0, 1, 0, 1, 1, 1, 0, 0},
  {1, 0, 1, 0, 0, 0, 0, 0},
  {1, 1, 0, 0, 0, 0, 0, 0},
};

unsigned char happiness_icon[8][8] =
{ {0, 0, 1, 1, 1, 1, 0, 0},
  {0, 1, 2, 2, 2, 2, 1, 0},
  {1, 2, 1, 2, 2, 1, 2, 1},
  {1, 2, 2, 2, 2, 2, 2, 1},
  {1, 2, 1, 2, 2, 1, 2, 1},
  {1, 2, 2, 1, 1, 2, 2, 1},
  {0, 1, 2, 2, 2, 2, 1, 0},
  {0, 0, 1, 1, 1, 1, 0, 0},
};

void erase_health() {
  display.fillRect(15, 5, 45, 8, WHITE);
}

void erase_energy() {
  display.fillRect(15, 15, 45, 8, WHITE);
}

void erase_hunger() {
  display.fillRect(80, 5, 45, 8, WHITE);
}

void erase_happiness() {
  display.fillRect(80, 15, 45, 8, WHITE);
}

void draw_stats() {
  // Health Bar
  display.drawRect(5, 7, 8, 4, BLACK);
  display.drawRect(7, 5, 4, 8, BLACK);
  display.fillRect(6, 8, 6, 2, RED);
  display.fillRect(8, 6, 2, 6, RED);
  display.fillRect(15, 5, (health / 100.0) * 45, 8, RED);
  display.drawRect(15, 5, 45, 8, BLACK);

  // Energy Bar
  for (unsigned char i = 0; i < 8; i++) {
    for (unsigned char j = 0; j < 8; j++) {
      if (lightning_icon[i][j] == 1) {
        display.drawPixel(5 + j, 15 + i, BLACK);
      } else if (lightning_icon[i][j] == 2) {
        display.drawPixel(5 + j, 15 + i, YELLOW);
      }
    }
  }
  display.fillRect(15, 15, (energy / 100.0) * 45, 8, YELLOW);
  display.drawRect(15, 15, 45, 8, BLACK);

  // Hunger Bar
  for (unsigned char i = 0; i < 8; i++) {
    for (unsigned char j = 0; j < 8; j++) {
      if (hunger_icon[i][j] == 1) {
        display.drawPixel(70 + j, 5 + i, BLACK);
      } else if (hunger_icon[i][j] == 2) {
        display.drawPixel(70 + j, 5 + i, ORANGE);
      }
    }
  }
  display.fillRect(80, 5, (hunger / 100.0) * 45, 8, ORANGE);
  display.drawRect(80, 5, 45, 8, BLACK);

  // Happiness Bar
  for (unsigned char i = 0; i < 8; i++) {
    for (unsigned char j = 0; j < 8; j++) {
      if (happiness_icon[i][j] == 1) {
        display.drawPixel(70 + j, 15 + i, BLACK);
      } else if (happiness_icon[i][j] == 2) {
        display.drawPixel(70 + j, 15 + i, LBLUE);
      }
    }
  }
  display.fillRect(80, 15, (happiness / 100.0) * 45, 8, LBLUE);
  display.drawRect(80, 15, 45, 8, BLACK);
}

unsigned char dropping[10][2];
unsigned char dropping_num = 0;

unsigned char dropping_icon[10][11] =
{ {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0},
  {0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0},
  {0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0},
  {0, 0, 1, 2, 2, 3, 2, 2, 1, 0, 0},
  {0, 1, 2, 2, 2, 2, 3, 3, 2, 1, 0},
  {1, 2, 2, 3, 2, 2, 2, 2, 2, 2, 1},
  {1, 2, 2, 2, 3, 3, 2, 2, 2, 2, 1},
  {0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0},
  {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0},
};

void Erase_Droppings() {
  for (unsigned char i = 0; i < dropping_num; i++) {
    display.fillRect(dropping[i][0], dropping[i][1], 10, 11, WHITE);
  }
}

void Draw_Droppings() {
  for (unsigned char k = 0; k < dropping_num; k++) {
    for (unsigned char i = 0; i < 10; i++) {
      for (unsigned char j = 0; j < 11; j++) {
        if (dropping_icon[i][j] == 1) {
          display.drawPixel(dropping[k][0] + j, dropping[k][1] + i, BLACK);
        } else if (dropping_icon[i][j] == 2) {
          display.drawPixel(dropping[k][0] + j, dropping[k][1] + i, BROWN);
        } else if (dropping_icon[i][j] == 3) {
          display.drawPixel(dropping[k][0] + j, dropping[k][1] + i, WHITE);
        }
      }
    }
  }
}

enum DR_States { DR_Drop } DR_State;

int Tick_Dropping(int state) {
  if (dropping_num >= 10) {
    Serial.println("dum >= 10");
    return;
  }
  Serial.println("x: " + x_pos);
  Serial.println("y: " + y_pos);
  dropping[dropping_num][0] = x_pos + 12;
  dropping[dropping_num][1] = y_pos + 21;
  dropping_num++;
  return state;
}

enum CH_States { CH_Pose1, CH_Pose2 } CH_State;

int Tick_Char(int state) {
  if (curr_scene != "main" && !move_enabled) {
    return state;
  }

  switch (state) {
    case CH_Pose1:
      state = CH_Pose2;
      break;
    case CH_Pose2:
      state = CH_Pose1;
      break;
    default:
      state = CH_Pose1;
      break;
  }

  switch (state) {
    case CH_Pose1:
      poseNum = 1;
      Erase_Char();
      Erase_Droppings();
      Draw_Char();
      Draw_Droppings();
      break;
    case CH_Pose2:
      poseNum = 2;
      Erase_Char();
      Erase_Droppings();
      Draw_Char();
      Draw_Droppings();
      break;
    default:
      break;
  }

  return state;
}

enum MV_States { MV_RandDirection, MV_Up, MV_Down, MV_Left, MV_Right } MV_State;
volatile unsigned char numTicks = 0;

int Tick_MoveChar(int state) {
  if (curr_scene != "main" && !move_enabled) {
    return state;
  }

  switch (state) {
    case MV_RandDirection:
      if (numTicks > 0) {
        state = MV_RandDirection;
      } else {
        state = random(1, 5);
        numTicks = random(3, 6);
      }
      break;
    case MV_Up:
      if (numTicks > 0) {
        state = MV_Up;
      } else {
        state = MV_RandDirection;
        numTicks = random(5, 9);
      }
      break;
    case MV_Down:
      if (numTicks > 0) {
        state = MV_Down;
      } else {
        state = MV_RandDirection;
        numTicks = random(5, 9);
      }
      break;
    case MV_Left:
      if (numTicks > 0) {
        state = MV_Left;
      } else {
        state = MV_RandDirection;
        numTicks = random(5, 9);
      }
      break;
    case MV_Right:
      if (numTicks > 0) {
        state = MV_Right;
      } else {
        state = MV_RandDirection;
        numTicks = random(5, 9);
      }
      break;
    default:
      state = MV_RandDirection;
      break;
  }

  switch (state) {
    case MV_RandDirection:
      numTicks--;
      break;
    case MV_Up:
      if (y_pos - pixel_size >= 26) {
        Erase_Char();
        Erase_Droppings();
        y_pos -= pixel_size;
        Draw_Char();
        Draw_Droppings();
      }
      numTicks--;
      break;
    case MV_Down:
      if (y_pos + char_size + pixel_size <= 96) {
        Erase_Char();
        Erase_Droppings();
        y_pos += pixel_size;
        Draw_Char();
      }
      numTicks--;
      break;
    case MV_Left:
      if (x_pos - pixel_size >= 3) {
        Erase_Char();
        Erase_Droppings();
        x_pos -= 3;
        Draw_Char();
        Draw_Droppings();
      }
      numTicks--;
      break;
    case MV_Right:
      if (x_pos + char_size + pixel_size <= 125) {
        Erase_Char();
        Erase_Droppings();
        x_pos += 3;
        Draw_Char();
        Draw_Droppings();
      }
      numTicks--;
      break;
    default:
      break;
  }

  return state;
}

void Erase_Char() {
  display.fillRect(x_pos, y_pos, char_size, char_size, WHITE);
}

void Draw_Char() {
  if (poseNum == 1) {
    for (unsigned char i = 0; i < char_pixel_size; i++) {
      for (unsigned char j = 0; j < char_pixel_size; j++) {
        if (pose1[i][j] != 0) {
          display.fillRect(x_pos + pixel_size * j, y_pos + pixel_size * i, pixel_size, pixel_size, BLACK);
        }
      }
    }
  } else if (poseNum == 2) {
    for (unsigned char i = 0; i < char_pixel_size; i++) {
      for (unsigned char j = 0; j < char_pixel_size; j++) {
        if (pose2[i][j] != 0) {
          display.fillRect(x_pos + pixel_size * j, y_pos + pixel_size * i, pixel_size, pixel_size, BLACK);
        }
      }
    }
  }
}

void Erase_SelectAction() {
  display.fillRect(0, 97, 128, 31, WHITE);

  display.drawRect(7, 97, 117, 28, BLACK);

  display.setCursor(12, 101);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.print("Clean");

  display.setCursor(90, 101);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.print("Sleep");

  display.setCursor(12, 113);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.print("Feed");

  display.setCursor(90, 113);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.print(" Play");
}

short x_joystick = 0;
short y_joystick = 0;

bool stationary() {
  x_joystick = analogRead(A0);
  y_joystick = analogRead(A1);

  if (abs(x_joystick - 512) < 200 && abs(y_joystick - 512) < 200) {
    return true;
  }
  return false;
}

String Get_Joystick_Direction() {
  x_joystick = analogRead(A0);
  y_joystick = analogRead(A1);

  if (stationary()) {
    return "Not Moving";
  }

  if (x_joystick >= 512) {
    if (y_joystick > 720) {
      return "D";
    } else if (y_joystick < 340) {
      return "U";
    } else {
      return "R";
    }
  } else {
    if (y_joystick > 680) {
      return "D";
    } else if (y_joystick < 300) {
      return "U";
    } else {
      return "L";
    }
  }

  return "Invalid";
}

enum SA_States { SA_Start, SA_Clean, SA_Sleep, SA_Feed, SA_Play } SA_State;

void Update_SelectAction(int state) {
  switch (state) {
    case SA_Clean:
      Erase_SelectAction();
      display.drawRect(9, 99, 35, 12, BLACK);
      break;
    case SA_Sleep:
      Erase_SelectAction();
      display.drawRect(87, 99, 35, 12, BLACK);
      break;
    case SA_Feed:
      Erase_SelectAction();
      display.drawRect(9, 111, 29, 12, BLACK);
      break;
    case SA_Play:
      Erase_SelectAction();
      display.drawRect(93, 111, 29, 12, BLACK);
      break;
    default:
      break;
  }
}

int Tick_SelectAction(int state) {
  if (curr_scene != "main") {
    return state;
  }

  String dir = Get_Joystick_Direction();

  switch (state) {
    case SA_Start:
      state = SA_Clean;
      Erase_SelectAction();
      Update_SelectAction(state);
      break;
    case SA_Clean:
      if (dir == "R") {
        state = SA_Sleep;
        Update_SelectAction(state);
      } else if (dir == "D") {
        state = SA_Feed;
        Update_SelectAction(state);
      }
      break;
    case SA_Sleep:
      if (dir == "L") {
        state = SA_Clean;
        Update_SelectAction(state);
      } else if (dir == "D") {
        state = SA_Play;
        Update_SelectAction(state);
      }
      break;
    case SA_Feed:
      if (dir == "U") {
        state = SA_Clean;
        Update_SelectAction(state);
      } else if (dir == "R") {
        state = SA_Play;
        Update_SelectAction(state);
      }
      break;
    case SA_Play:
      if (dir == "U") {
        state = SA_Sleep;
        Update_SelectAction(state);
      } else if (dir == "L") {
        state = SA_Feed;
        Update_SelectAction(state);
      }
      break;
    default:
      SA_State = SA_Start;
      Update_SelectAction(state);
      break;
  }

  switch (state) {
    case SA_Start:
      break;
    case SA_Clean:
      if (digitalRead(6) == LOW) {
        curr_scene = "clean";
        display.fillRect(0, 26, 128, 102, WHITE);
        display.setCursor(12, 113);
        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.print("Press to Clean...");
        move_enabled = true;
        Draw_Char();
        Draw_Droppings();
      }
      break;
    case SA_Sleep:
      if (digitalRead(6) == LOW) {
        curr_scene = "sleep";
        display.fillRect(0, 26, 128, 102, BLUE);
        display.setCursor(12, 101);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.print("Sleeping...");
        display.setCursor(12, 113);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.print("Press to Wake...");
        poseNum = 2;
        pose2[8][4] = 1;
        pose2[8][10] = 1;
        Draw_Char();
      }
      break;
    case SA_Feed:
      if (digitalRead(6) == LOW) {
        curr_scene = "feed";
        display.fillRect(0, 26, 128, 102, WHITE);
        display.setCursor(12, 113);
        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.print("Press to Feed...");
        x_pos = 20;
        y_pos = 50;
        poseNum = 1;
        pose1[8][7] = 1;
        pose1[9][7] = 0;
        pose1[10][6] = 1;
        pose1[10][7] = 1;
        pose1[10][8] = 1;
        Draw_Char();
      }
      break;
    case SA_Play:
      if (digitalRead(6) == LOW) {
        curr_scene = "play";
        display.fillRect(0, 26, 128, 102, WHITE);
        display.setCursor(12, 113);
        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.print("Left or Right...");
        display.setCursor(10, 40);
        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.print("Left");
        display.setCursor(92, 40);
        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.print("Right");
        x_pos = 48;
        y_pos = 52;
        poseNum = 1;
        Draw_Char();
      }
      break;
    default:
      break;
  }

  return state;
}

enum CL_States { CL_WaitRelease, CL_WaitPress, CL_Clean, CL_Reset } CL_State;

int Tick_Clean(int state) {
  if (curr_scene != "clean") {
    return state;
  }

  switch(state) {
    case CL_WaitRelease:
      if (digitalRead(6) == HIGH) {
        state = CL_WaitPress;
      }
      break;
    case CL_WaitPress:
      if (digitalRead(6) == LOW) {
        state = CL_Clean;
      }
      break;
    case CL_Clean:
      dropping_num = 0;
      health += 50;
      if (health > 100) {
        health = 100;
      }
      state = CL_Reset;
      break;
    case CL_Reset:
      display.fillRect(0, 26, 128, 102, WHITE);
      move_enabled = false;
      state = CL_WaitRelease;
      curr_scene = "main";
      Erase_SelectAction();
      Update_SelectAction(1);
      break;
    default:
      state = CL_WaitRelease;
      break;
  }

  return state;
}

void reset_pose2_sleep() {
  pose2[8][4] = 0;
  pose2[8][10] = 0;
}

enum SP_States { SP_WaitRelease, SP_Sleep, SP_Wake } SP_State;

int Tick_Sleep(int state) {
  if (curr_scene != "sleep") {
    return state;
  }
  
  switch(state) {
    case SP_WaitRelease:
      if (digitalRead(6) == HIGH) {
        state = SP_Sleep;
      }
      break;
    case SP_Sleep:
      if (digitalRead(6) == LOW || energy >= 99) {
        state = SP_Wake;
      }
      hunger_decay_amount = 0.5;
      energy_decay_amount = 0;
      energy += 0.2;
      break;
    case SP_Wake:
      state = SP_WaitRelease;
      curr_scene = "main";
      reset_pose2_sleep();
      display.fillRect(0, 26, 128, 102, WHITE);
      Erase_SelectAction();
      Update_SelectAction(2);
      hunger_decay_amount = 1;
      energy_decay_amount = 1;
      break;
    default:
      break;
  }

  return state;
}

void reset_pose1_feed() {
  pose1[8][7] = 0;
  pose1[9][7] = 1;
  pose1[10][6] = 0;
  pose1[10][7] = 0;
  pose1[10][8] = 0;
}

enum FD_States { FD_WaitRelease, FD_WaitPress, FD_ShootPellet, FD_Reset } FD_State;

unsigned char pellet_loc;

int Tick_Feed(int state) {
  if (curr_scene != "feed") {
    return state;
  }
  
  switch(state) {
    case FD_WaitRelease:
      if (digitalRead(6) == HIGH) {
        state = FD_WaitPress;
      }
      break;
    case FD_WaitPress:
      display.fillRect(41, 77, 3, 3, WHITE);
      if (digitalRead(6) == LOW) {
        state = FD_ShootPellet;
        pellet_loc = 113;
      }
      break;
    case FD_ShootPellet:
      if (pellet_loc <= 41) {
        hunger += 20;
        if (hunger > 100) {
          hunger = 100;
        }
        state = FD_Reset;
      }
      display.fillRect(pellet_loc + 6, 77, 3, 3, WHITE);
      display.fillRect(pellet_loc, 77, 3, 3, ORANGE);
      pellet_loc -= 6;
      break;
    case FD_Reset:
      state = FD_WaitRelease;
      curr_scene = "main";
      reset_pose1_feed();
      Erase_SelectAction();
      Update_SelectAction(3);
      break;
    default:
      break;
  }

  return state;
}

enum PL_States { PL_WaitRelease, PL_WaitPressLeft, PL_WaitPressRight, PL_Result, PL_Reset } PL_State;

String choice;
unsigned char move_frames = 10;
String answer;

int Tick_Play(int state) {
  if (curr_scene != "play") {
    return state;
  }
  
  switch(state) {
    case PL_WaitRelease:
      if (digitalRead(6) == HIGH) {
        state = PL_WaitPressLeft;
      }
      break;
    case PL_WaitPressLeft:
      display.drawRect(7, 38, 29, 12, BLACK);
      if (Get_Joystick_Direction() == "R") {
        state = PL_WaitPressRight;
        display.drawRect(7, 38, 29, 12, WHITE);
      } else if (digitalRead(6) == LOW) {
        choice = "L";
        state = PL_Result;
        answer = random(0, 2) == 0 ? "L" : "R";
      }
      break;
    case PL_WaitPressRight:
      display.drawRect(89, 38, 35, 12, BLACK);
      if (Get_Joystick_Direction() == "L") {
        state = PL_WaitPressLeft;
        display.drawRect(89, 38, 35, 12, WHITE);
      } else if (digitalRead(6) == LOW) {
        choice = "R";
        state = PL_Result;
        answer = random(0, 2) == 0 ? "L" : "R";
      }
      break;
    case PL_Result:
      Erase_Char();
      if (answer == "L") {
        x_pos -= 3;
      } else {
        x_pos += 3;
      }
      Draw_Char();
      if (move_frames <= 8) {
        display.setCursor(12, 101);
        display.setTextSize(1);
        display.setTextColor(BLACK);
        if (choice == answer) {
          display.print("Correct!");
        } else {
          display.print("Wrong :(");
        }
      }
      if (move_frames <= 0) {
        happiness += 20;
        if (happiness > 100) {
          happiness = 100;
        }
        state = PL_Reset;
      }
      move_frames--;
      break;
    case PL_Reset:
      display.fillRect(0, 38, 128, 12, WHITE);
      state = PL_WaitRelease;
      move_frames = 10;
      curr_scene = "main";
      Erase_SelectAction();
      Update_SelectAction(4);
      break;
    default:
      break;
  }

  return state;
}

String life_length;

void Draw_Dead() {
  display.fillRect(0, 38, 128, 90, WHITE);
  
  display.setCursor(12, 101);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.print("RIP...");

  int minutes = (millis() - birth_time) / 60000;
  display.setCursor(12, 113);
  display.print("Minutes: ");
  
  display.setCursor(62, 113);
  display.print(minutes);
  
  Erase_Char();
  pose1[8][6] = 0;
  pose1[8][8] = 0;
  pose1[5][4] = 1;
  pose1[5][6] = 1;
  pose1[7][4] = 1;
  pose1[7][6] = 1;
  pose1[5][8] = 1;
  pose1[5][10] = 1;
  pose1[7][8] = 1;
  pose1[7][10] = 1;
  Draw_Char();
}

enum HP_States { HP_Drop } HP_State;

int Tick_HealthDrop(int state) {
  if (curr_scene == "dead") {
    return state;
  }
  if (health <= 0) {
    curr_scene = "dead";
    Draw_Dead();
  } else if (energy <= 0 || hunger <= 0 || happiness <= 0 || dropping_num >= 5) {
    health--;
  } else if (energy >= 75 && hunger >= 75 && dropping_num <= 2) {
    if (health < 100) {
      health++;
    }
  }
  erase_health();
  draw_stats();
  return state;
}

enum EG_States { EG_Decay } EG_State;

int Tick_EnergyDecay(int state) {
  if (curr_scene == "dead") {
    return state;
  }
  if (energy > 0) {
    energy -= energy_decay_amount;
  }
  erase_energy();
  draw_stats();
  return state;
}

enum HG_States { HG_Decay } HG_State;

int Tick_HungerDecay(int state) {
  if (curr_scene == "dead") {
    return state;
  }
  if (hunger > 0) {
    hunger -= hunger_decay_amount;
  }
  erase_hunger();
  draw_stats();
  return state;
}

enum HN_States { HN_Drop } HN_State;

int Tick_HappinessDrop(int state) {
  if (curr_scene == "dead") {
    return state;
  }
  if ((hunger <= 25 || energy <= 15) && happiness > 0) {
    happiness--;
  }
  if (health <= 25 && happiness > 0) {
    happiness--;
  }
  erase_happiness();
  draw_stats();
  return state;
}

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(6, INPUT);
  digitalWrite(6, HIGH);

  display.begin();
  display.clearScreen();
  display.fillRect(0, 0, 128, 128, WHITE);

  draw_stats();
  
  unsigned char i = 0;
  tasks[i].state = CH_Pose1; // 1
  tasks[i].period = 1000;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Tick_Char;
  ++i;
  tasks[i].state = MV_RandDirection; // 2
  tasks[i].period = 500;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Tick_MoveChar;
  ++i;
  tasks[i].state = SA_Start; // 3
  tasks[i].period = 200;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Tick_SelectAction;
  ++i;
  tasks[i].state = HP_Drop; // 4
  tasks[i].period = 1000;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Tick_HealthDrop;
  ++i;
  tasks[i].state = EG_Decay; // 5
  tasks[i].period = 2000;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Tick_EnergyDecay;
  ++i;
  tasks[i].state = HG_Decay; // 6
  tasks[i].period = 3000;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Tick_HungerDecay;
  ++i;
  tasks[i].state = HN_Drop; // 7
  tasks[i].period = 2000;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Tick_HappinessDrop;
  ++i;
  tasks[i].state = FD_WaitRelease; // 8
  tasks[i].period = 100;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Tick_Feed;
  ++i;
  tasks[i].state = SP_WaitRelease; // 9
  tasks[i].period = 200;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Tick_Sleep;
  ++i;
  tasks[i].state = DR_Drop; // 10
  tasks[i].period = 20000;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Tick_Dropping;
  ++i;
  tasks[i].state = CL_WaitRelease; // 11
  tasks[i].period = 200;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Tick_Clean;
  ++i;
  tasks[i].state = PL_WaitRelease; // 12
  tasks[i].period = 200;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Tick_Play;
  ++i;
}

void loop() {
  for (unsigned char i = 0; i < numTasks; i++) {
    if (millis() - tasks[i].elapsedTime >= tasks[i].period) {
      tasks[i].state = tasks[i].TickFct(tasks[i].state);
      tasks[i].elapsedTime = millis();
    }
  }
}
