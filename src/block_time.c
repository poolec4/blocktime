#include <pebble.h>
#include <ctype.h>

char background_color_hex_char[10] = "000000"; 
char font_color_hex_char[10] = "FFFFFF";

int vibrate_status = 2;
int temp_units = 0;
int location_status = 0;
int zip_code_int = 0;
int background_color_hex_int;
int font_color_hex_int;
int temp_to_store = 0;
int middle_outline_status = 1;
int update_interval_val = 60;
int sleep_status = 0;
int sleep_start_hour = 0;
int sleep_end_hour = 0;
int battery_meter_status = 1;

char latitude_to_store[15] = "0";
char longitude_to_store[15] = "0";

bool is_in_sleep = false;

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_POS 2

//settings keys
#define KEY_BACKGROUND_COLOR 3
#define KEY_FONT_COLOR 4
#define KEY_VIBRATE 5
#define KEY_UNIT 6
#define KEY_LOCATION 7
#define KEY_ZIP_CODE 8
#define KEY_MIDDLE_OUTLINE 9
#define KEY_LATITUDE 10
#define KEY_LONGITUDE 11
#define KEY_UPDATE_INTERVAL 12
#define KEY_SLEEP 13
#define KEY_SLEEP_START 14
#define KEY_SLEEP_END 15
#define KEY_BATTERY_METER 16

static AppSync s_sync;
static uint8_t s_sync_buffer[500];

#define MyTupletCString(_key, _cstring) \
((const Tuplet) { .type = TUPLE_CSTRING, .key = _key, .cstring = { .data = _cstring, .length = strlen(_cstring) + 1 }})

static Window *window;
static Layer *canvas_layer;
static Layer *text_layer;
static TextLayer *month_text_layer;
static TextLayer *day_text_layer;
static TextLayer *temperature_text_layer;
static TextLayer *day_of_week_text_layer;
static TextLayer *sleep_text_layer;

static char day_buffer[10];
static char date_buffer[10];
static char month_buffer[10];
static char temperature_buffer[8];

GColor backgroundGColor;
GColor fontGColor;

static int digit_matrix[10][5][3] = 
{
  {
    {1, 1, 1}, // 0
    {1, 0, 1},
    {1, 0, 1},
    {1, 0, 1},
    {1, 1, 1}
  },
  {
    {0, 1, 0}, // 1
    {1, 1, 0},
    {0, 1, 0},
    {0, 1, 0},
    {1, 1, 1}
  },
  {
    {1, 1, 1}, // 2
    {0, 0, 1},
    {1, 1, 1},
    {1, 0, 0},
    {1, 1, 1}
  },
  {
    {1, 1, 1}, // 3
    {0, 0, 1},
    {1, 1, 1},
    {0, 0, 1},
    {1, 1, 1}
  },
  {
    {1, 0, 1}, // 4
    {1, 0, 1},
    {1, 1, 1},
    {0, 0, 1},
    {0, 0, 1}
  },
  {
    {1, 1, 1}, // 5
    {1, 0, 0},
    {1, 1, 1},
    {0, 0, 1},
    {1, 1, 1}
  },
  {
    {1, 1, 1}, // 6
    {1, 0, 0},
    {1, 1, 1},
    {1, 0, 1},
    {1, 1, 1}
  },
  {
    {1, 1, 1}, // 7
    {0, 0, 1},
    {0, 0, 1},
    {0, 0, 1},
    {0, 0, 1}
  },
  {
    {1, 1, 1}, // 8
    {1, 0, 1},
    {1, 1, 1},
    {1, 0, 1},
    {1, 1, 1}
  },
  {
    {1, 1, 1}, // 9
    {1, 0, 1},
    {1, 1, 1},
    {0, 0, 1},
    {1, 1, 1}
  } 
};

static int digit_rounded_corners[10][5][3] = 
{
  {
    {1, 0, 2}, // 0
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {4, 0, 3}
  },
  {
    {0, 7, 0}, // 1
    {5, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {5, 0, 6}
  },
  {
    {5, 0, 2}, // 2
    {0, 0, 0},
    {1, 0, 3},
    {0, 0, 0},
    {4, 0, 6}
  },
  {
    {5, 0, 2}, // 3
    {0, 0, 0},
    {5, 0, 0},
    {0, 0, 0},
    {5, 0, 3}
  },
  {
    {7, 0, 7}, // 4
    {0, 0, 0},
    {4, 0, 0},
    {0, 0, 0},
    {0, 0, 8}
  },
  {
    {1, 0, 6}, // 5
    {0, 0, 0},
    {4, 0, 2},
    {0, 0, 0},
    {5, 0, 3}
  },
  {
    {1, 0, 6}, // 6
    {0, 0, 0},
    {0, 0, 2},
    {0, 0, 0},
    {4, 0, 3}
  },
  {
    {5, 0, 2}, // 7
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 8}
  },
  {
    {1, 0, 2}, // 8
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {4, 0, 3}
  },
  {
    {1, 0, 2}, // 9
    {0, 0, 0},
    {4, 0, 0},
    {0, 0, 0},
    {5, 0, 3}
  } 
};


unsigned int HexStringToUInt(char const* hexstring)
{
  unsigned int result = 0;
  char const *c = hexstring;
  unsigned char thisC;
  while( (thisC = *c) != 0 )
  {
    thisC = toupper(thisC);
    result <<= 4;
    if( isdigit(thisC))
      result += thisC - '0';
    else if(isxdigit(thisC))
      result += thisC - 'A' + 10;
    else
    {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: Unrecognized hex character \"%c\"", thisC);
      return 0;
    }
    ++c;
  }
  return result;  
}

static void requestWeather(void) 
{
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (!iter) {
    // Error creating outbound message
    return;
  }

  int value = 1;
  dict_write_int(iter, 1, &value, sizeof(int), true);
  dict_write_end(iter);

  app_message_outbox_send();
}

int numbers_to_print[4] = {0,0,0,0};
int x_start[4] = {2, 83, 2, 83};
int y_start[4] = {2, 2, 97, 97};

static void update_time()
{
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  int normal_hours = 0;

  if (clock_is_24h_style())
  {
    numbers_to_print[0] = tick_time->tm_hour/10;
    numbers_to_print[1] = tick_time->tm_hour-(10*numbers_to_print[0]);
  }
  else
  {
    normal_hours = tick_time->tm_hour;

    if (tick_time->tm_hour > 12)
    {    
      normal_hours = tick_time->tm_hour-12;
    }
    if (tick_time->tm_hour == 0)
    {
      normal_hours = 12;
    }

    numbers_to_print[0] = (normal_hours/10);
    numbers_to_print[1] = (normal_hours-(10*numbers_to_print[0]));
  }

  numbers_to_print[2] = tick_time->tm_min/10;
  numbers_to_print[3] = tick_time->tm_min-(10*numbers_to_print[2]);

  strftime(date_buffer, sizeof("**"), "%e",tick_time);
  strftime(day_buffer, sizeof("***"), "%a",tick_time);

  switch(tick_time->tm_mon)
  {
    case 0:
      strcpy(month_buffer,"Jan");
      break;
    case 1:
      strcpy(month_buffer,"Feb");
      break;
    case 2:
      strcpy(month_buffer,"Mar");
      break;
    case 3:
      strcpy(month_buffer,"Apr");
      break;
    case 4:
      strcpy(month_buffer,"May");
      break;
    case 5:
      strcpy(month_buffer,"June");
      break;
    case 6:
      strcpy(month_buffer,"July");
      break;
    case 7:
      strcpy(month_buffer,"Aug");
      break;
    case 8:
      strcpy(month_buffer,"Sep");
      break;
    case 9:
      strcpy(month_buffer,"Oct");
      break;
    case 10:
      strcpy(month_buffer,"Nov");
      break;
    case 11:
      strcpy(month_buffer,"Dec");
      break;
  }

  text_layer_set_text(day_text_layer, date_buffer);
  text_layer_set_text(day_of_week_text_layer, day_buffer);
  text_layer_set_text(month_text_layer, month_buffer);

  if(canvas_layer)
    layer_mark_dirty(canvas_layer);

  if (sleep_status == 1)
  {
    if( (tick_time->tm_hour >= sleep_start_hour || sleep_start_hour > sleep_end_hour) && (tick_time->tm_hour < sleep_end_hour))
      is_in_sleep = true;
    else
      is_in_sleep = false;
  }
  else        
  {
    is_in_sleep = false;
  }

  if (is_in_sleep == true)
  {    
    text_layer_set_text(sleep_text_layer, "Sleep");
    text_layer_set_text(temperature_text_layer, "");
  }
  else
  {        
    text_layer_set_text(sleep_text_layer, "");
    if(temp_to_store < 100)
    {
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°", temp_to_store);
    }
    else
    {
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%d", temp_to_store);
    }
    text_layer_set_text(temperature_text_layer, temperature_buffer);
  }

  if(tick_time->tm_min % update_interval_val == 0 && tick_time->tm_sec % 60 == 0 && is_in_sleep != true)
  {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, KEY_UNIT, temp_units);  
    dict_write_uint8(iter, KEY_LOCATION, location_status);  
    dict_write_uint32(iter, KEY_ZIP_CODE, zip_code_int);  
    dict_write_cstring(iter, KEY_LATITUDE, latitude_to_store);
    dict_write_cstring(iter, KEY_LONGITUDE, longitude_to_store);  

    requestWeather();
  }


  if(tick_time->tm_min % 60 == 0 && tick_time->tm_sec % 60 == 0 && is_in_sleep != true)
  {
    switch (vibrate_status)
    {
      case 0:
        vibes_short_pulse();
        break;
      case 1:
        vibes_long_pulse();
        break;
      default:
        break;
    }
  }
}

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) 
{
  if (app_message_error != 0)
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* t, const Tuple* old_tuple, void* context) 
{
  static char conditions_buffer[32];
  static char location_buffer[32];

  APP_LOG(APP_LOG_LEVEL_INFO, "AppMessage of key %d Received", (int)t->key);

  switch (key) {
    case KEY_TEMPERATURE:
      temp_to_store = (int)t->value->int32;
      if(temp_to_store < 100)
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°", (int)t->value->int32);
      else
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%d", (int)t->value->int32);
      break;

    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;

    case KEY_POS:
      snprintf(location_buffer, sizeof(location_buffer), "%s", t->value->cstring);
      break;

    case KEY_BACKGROUND_COLOR:
      snprintf(background_color_hex_char, sizeof(background_color_hex_char), "%s", t->value->cstring);    
      break;

    case KEY_FONT_COLOR:
      snprintf(font_color_hex_char, sizeof(font_color_hex_char), "%s", t->value->cstring);    
      break;

    case KEY_VIBRATE:
      vibrate_status = (int)t->value->int32;
      break;

    case KEY_UNIT:
      temp_units = (int)t->value->int32;
      break;
    
    case KEY_LOCATION:
      location_status = (int)t->value->int32;
      break;      
      
    case KEY_ZIP_CODE:
      zip_code_int = (int)t->value->int32;
      break;
    
    case KEY_MIDDLE_OUTLINE:
      middle_outline_status = (int)t->value->int32;
      break;
      
    case KEY_LATITUDE:
      snprintf(latitude_to_store, sizeof(latitude_to_store), "%s", t->value->cstring);
      break;
      
    case KEY_LONGITUDE:
      snprintf(longitude_to_store, sizeof(longitude_to_store), "%s", t->value->cstring);
      break;
    
    case KEY_UPDATE_INTERVAL:
      update_interval_val = (int)t->value->int32;
      break;
    
    case KEY_SLEEP:
      sleep_status = (int)t->value->int32;
      break;
    
    case KEY_SLEEP_START:
      sleep_start_hour = (int)t->value->int32;
      break;
    
    case KEY_SLEEP_END:
      sleep_end_hour = (int)t->value->int32;
      break;
    
    case KEY_BATTERY_METER:
      battery_meter_status = (int)t->value->int32;
      APP_LOG(APP_LOG_LEVEL_INFO, "battery_meter: %d", battery_meter_status);
      break;
    
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key%d not recognized", (int)t->key);
      break;
  }

  text_layer_set_font(temperature_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  if (is_in_sleep != true)    
    text_layer_set_text(temperature_text_layer, temperature_buffer);

  text_layer_set_font(temperature_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

  #ifdef PBL_COLOR
    text_layer_set_text_color(month_text_layer, GColorFromHEX(font_color_hex_int));
    text_layer_set_text_color(day_text_layer, GColorFromHEX(font_color_hex_int));
    text_layer_set_text_color(temperature_text_layer, GColorFromHEX(font_color_hex_int));
    text_layer_set_text_color(sleep_text_layer, GColorFromHEX(font_color_hex_int));
    text_layer_set_text_color(day_of_week_text_layer, GColorFromHEX(font_color_hex_int));
  #else
    text_layer_set_text_color(month_text_layer, fontGColor);
    text_layer_set_text_color(day_text_layer, fontGColor);
    text_layer_set_text_color(temperature_text_layer, fontGColor);
    text_layer_set_text_color(sleep_text_layer, fontGColor);
    text_layer_set_text_color(day_of_week_text_layer, fontGColor);
  #endif
  
  layer_mark_dirty(canvas_layer);
  layer_mark_dirty(text_layer);

  update_time();
}

static void canvas_layer_update_proc(Layer *this_layer, GContext *ctx)
{
  int x_offset = 19;
  int y_offset = 13;

  static int corner_radius = 3;

  background_color_hex_int = HexStringToUInt(background_color_hex_char);
  font_color_hex_int = HexStringToUInt(font_color_hex_char);

  switch(background_color_hex_int)
  {
    case 0:
      backgroundGColor = GColorBlack;
      break;
    case 16777215: 
      backgroundGColor = GColorWhite;
      break;
    default:
      backgroundGColor = GColorBlack;
      break;
  }

  switch(font_color_hex_int)
  {
    case 0:
      fontGColor = GColorBlack;
      break;
    case 16777215:
      fontGColor = GColorWhite;
      break;
    default:
      fontGColor = GColorWhite;
      break;
  }

  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorFromHEX(background_color_hex_int));
  #else  
    graphics_context_set_fill_color(ctx, backgroundGColor);
  #endif

    graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);

  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorFromHEX(font_color_hex_int));
  #else
    graphics_context_set_fill_color(ctx, fontGColor);
  #endif

  for (int x=0; x<3; x++)
  {
    for (int y=0; y<5; y++)
    {
      for (int k=0; k<4; k++)
      {
        if (digit_matrix[numbers_to_print[k]][y][x] == 1)
        {
          switch(digit_rounded_corners[numbers_to_print[k]][y][x])
            {
            case 0:
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornerNone);  
            break;
            case 1:
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornerTopLeft);  
            break;
            case 2: 
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornerTopRight);  
            break;
            case 3:
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornerBottomRight);  
            break;
            case 4: 
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornerBottomLeft);  
            break;
            case 5:
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornersLeft);  
            break;
            case 6:
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornersRight);  
            break;
            case 7:
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornersTop);  
            break;
            case 8:
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornersBottom);  
            break;
            }
          }
        }
      }
    }

  if (battery_meter_status == 1)
  {
    BatteryChargeState charge_state = battery_state_service_peek();

    double x_length_1 = 25;

    switch (charge_state.charge_percent)
    {
      case 0:
      x_length_1 = 2.;
      break;
      case 5:
      x_length_1 = 5.;
      break;
      case 10:
      x_length_1 = 8.4;
      break;
      case 20:
      x_length_1 = 16.8;
      break;
      case 30:
      x_length_1 = 25.;
      break;
      case 40:
      x_length_1 = 8.4;
      break;
      case 50:
      x_length_1 = 16.8;
      break;
      case 60:
      x_length_1 = 25.;
      break;
      case 70:
      x_length_1 = 6.25;
      break;
      case 80:
      x_length_1 = 12.5;
      break;
      case 90:
      x_length_1 = 18.75;
      break;
      case 100:
      x_length_1 = 25.;
      break;
    }

    if (charge_state.charge_percent <= 30)
    {      
      if (middle_outline_status == 1)
      {
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_fill_rect(ctx, GRect(68, 0, 8, 1+x_length_1), 0, GCornersAll);
        graphics_fill_rect(ctx, GRect(68, 167-x_length_1, 8, 26 + x_length_1), 0, GCornersAll);          
      }

      #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorRed);
      #else
      graphics_context_set_fill_color(ctx, fontGColor);
      #endif

      graphics_fill_rect(ctx, GRect(69, 0, 6, x_length_1), 0, GCornersAll);
      graphics_fill_rect(ctx, GRect(69, 168-x_length_1, 6, x_length_1), 0, GCornersAll);  
    }

    if (charge_state.charge_percent > 30 && charge_state.charge_percent < 70)
    {
      if (middle_outline_status == 1)
      {
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_fill_rect(ctx, GRect(68, 0, 8, 26+x_length_1), 0, GCornersAll);
        graphics_fill_rect(ctx, GRect(68, 142-x_length_1, 8, 51 + x_length_1), 0, GCornersAll);          
      }

      #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorRed);
      #else
      graphics_context_set_fill_color(ctx, fontGColor);
      #endif
      graphics_fill_rect(ctx, GRect(69, 0, 6, 25), 0, GCornersAll);
      graphics_fill_rect(ctx, GRect(69, 143, 6, 25), 0, GCornersAll);  
      #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorYellow);
      #else
      graphics_context_set_fill_color(ctx, fontGColor);
      #endif
      graphics_fill_rect(ctx, GRect(69, 25, 6, x_length_1), 0, GCornersAll);
      graphics_fill_rect(ctx, GRect(69, 143-x_length_1, 6, x_length_1), 0, GCornersAll);  
    }

    if (charge_state.charge_percent >= 70)
    {
      if (middle_outline_status == 1)
      {
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_fill_rect(ctx, GRect(68, 0, 8, 51+x_length_1), 0, GCornersAll);
        graphics_fill_rect(ctx, GRect(68, 117-x_length_1, 8, 76 + x_length_1), 0, GCornersAll);          
      }

      #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorRed);
      #else
      graphics_context_set_fill_color(ctx, fontGColor);
      #endif
      graphics_fill_rect(ctx, GRect(69, 0, 6, 25), 0, GCornersAll);
      graphics_fill_rect(ctx, GRect(69, 143, 6, 25), 0, GCornersAll);  
      #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorYellow);
      #else
      graphics_context_set_fill_color(ctx, fontGColor);
      #endif
      graphics_fill_rect(ctx, GRect(69, 25, 6, 25), 0, GCornersAll);
      graphics_fill_rect(ctx, GRect(69, 118, 6, 25), 0, GCornersAll);  
      #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorGreen);
      #else
      graphics_context_set_fill_color(ctx, fontGColor);
      #endif
      graphics_fill_rect(ctx, GRect(69, 50, 6, x_length_1), 0, GCornersAll); 
      graphics_fill_rect(ctx, GRect(69, 118-x_length_1, 6, x_length_1), 0, GCornersAll);   
    }

    if (charge_state.is_charging)
    {
      if (middle_outline_status == 1)
      {
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_fill_rect(ctx, GRect(68, 0, 8, 76), 0, GCornersAll);
        graphics_fill_rect(ctx, GRect(68, 92, 8, 76), 0, GCornersAll);
      }

      #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorGreen);
      #else
      graphics_context_set_fill_color(ctx, fontGColor);
      #endif
      graphics_fill_rect(ctx, GRect(69, 0, 6, 75), 0, GCornersAll);  
      graphics_fill_rect(ctx, GRect(69, 93, 6, 75), 0, GCornersAll);  
    }
  }
  else
  {
    if (middle_outline_status == 1)
    {
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_rect(ctx, GRect(68, 0, 8, 76), 0, GCornersAll);  
    }

    #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorRed);
      #else
    graphics_context_set_fill_color(ctx, fontGColor);
      #endif
    graphics_fill_rect(ctx, GRect(69, 0, 6, 25), 0, GCornersAll);
    #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorYellow);
      #else
    graphics_context_set_fill_color(ctx, fontGColor);
      #endif
    graphics_fill_rect(ctx, GRect(69, 25, 6, 25), 0, GCornersAll);
    #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorGreen);
      #else
    graphics_context_set_fill_color(ctx, fontGColor);
      #endif
    graphics_fill_rect(ctx, GRect(69, 50, 6, 25), 0, GCornersAll); 
    
    if (middle_outline_status == 1)
    {
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_rect(ctx, GRect(68, 92, 8, 76), 0, GCornersAll);  
    }

    #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorGreen);
      #else
    graphics_context_set_fill_color(ctx, fontGColor);
      #endif
    graphics_fill_rect(ctx, GRect(69, 93, 6, 25), 0, GCornersAll);
    #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorYellow);
      #else
    graphics_context_set_fill_color(ctx, fontGColor);
      #endif
    graphics_fill_rect(ctx, GRect(69, 118, 6, 25), 0, GCornersAll);
    #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorRed);
      #else
    graphics_context_set_fill_color(ctx, fontGColor);
      #endif
    graphics_fill_rect(ctx, GRect(69, 143, 6, 25), 0, GCornersAll);  
  }

  if (middle_outline_status == 1)
  {
    #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorBlack);
    #else
      graphics_context_set_fill_color(ctx, fontGColor);
    #endif
      graphics_fill_rect(ctx, GRect(65, 77, 14, 14), 1, GCornersAll);  
  }

  if (bluetooth_connection_service_peek())
  {
    #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorBlueMoon);
    #else
      graphics_context_set_fill_color(ctx, fontGColor);
    #endif
  }
  else
  {
    #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorRed);
      #else
    graphics_context_set_fill_color(ctx, GColorBlack);
      #endif
  }
  graphics_fill_rect(ctx, GRect(66, 78, 12, 12), 1, GCornersAll);  

  #ifdef PBL_COLOR
    text_layer_set_text_color(month_text_layer, GColorFromHEX(font_color_hex_int));
    text_layer_set_text_color(day_text_layer, GColorFromHEX(font_color_hex_int));
    text_layer_set_text_color(temperature_text_layer, GColorFromHEX(font_color_hex_int));
    text_layer_set_text_color(sleep_text_layer, GColorFromHEX(font_color_hex_int));
    text_layer_set_text_color(day_of_week_text_layer, GColorFromHEX(font_color_hex_int));
  #else
    text_layer_set_text_color(month_text_layer, fontGColor);
    text_layer_set_text_color(day_text_layer, fontGColor);
    text_layer_set_text_color(temperature_text_layer, fontGColor);
    text_layer_set_text_color(sleep_text_layer, fontGColor);
    text_layer_set_text_color(day_of_week_text_layer, fontGColor);
  #endif
}

static void window_load(Window *window) 
{
  Layer *window_layer = window_get_root_layer(window);
  text_layer = window_get_root_layer(window);

  canvas_layer = layer_create(GRect(0, 0, 144, 168));
  layer_add_child(window_layer, canvas_layer);
  layer_set_update_proc(canvas_layer, canvas_layer_update_proc);

  month_text_layer = text_layer_create(GRect(82, 67, 72, 30));
  text_layer_set_text_alignment(month_text_layer, GTextAlignmentLeft);
  text_layer_set_background_color(month_text_layer, GColorClear);

  
    text_layer_set_font(month_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  //layer_add_child(window_get_root_layer(window), text_layer_get_layer(month_text_layer));
    layer_add_child(text_layer, text_layer_get_layer(month_text_layer));

    day_text_layer = text_layer_create(GRect(76, 67, 64, 30));
    text_layer_set_text_alignment(day_text_layer, GTextAlignmentRight);
    text_layer_set_background_color(day_text_layer, GColorClear);
  
    text_layer_set_font(day_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  //layer_add_child(window_get_root_layer(window), text_layer_get_layer(day_text_layer));
    layer_add_child(text_layer, text_layer_get_layer(day_text_layer));

    temperature_text_layer = text_layer_create(GRect(2, 67, 64, 30));
    text_layer_set_text_alignment(temperature_text_layer, GTextAlignmentLeft);
    text_layer_set_background_color(temperature_text_layer, GColorClear);
  
    text_layer_set_font(temperature_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));

    static char temp[10];

    if(temp_to_store < 100)
    {
      snprintf(temp, sizeof(temp), "%d°", temp_to_store);
    }
    else
    {
      snprintf(temp, sizeof(temp), "%d", temp_to_store);
    }

    if (temp_to_store == 0)
    {
      snprintf(temp, sizeof(temp), "Load");
    }

    text_layer_set_text(temperature_text_layer, temp);
  //layer_add_child(window_get_root_layer(window), text_layer_get_layer(temperature_text_layer));
    layer_add_child(text_layer, text_layer_get_layer(temperature_text_layer));

    sleep_text_layer = text_layer_create(GRect(1, 71, 64, 30));
    text_layer_set_text_alignment(sleep_text_layer, GTextAlignmentLeft);
    text_layer_set_background_color(sleep_text_layer, GColorClear);
  
    text_layer_set_font(sleep_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    layer_add_child(text_layer, text_layer_get_layer(sleep_text_layer));

    day_of_week_text_layer = text_layer_create(GRect(0, 67, 62, 30));
    text_layer_set_text_alignment(day_of_week_text_layer, GTextAlignmentRight);
    text_layer_set_background_color(day_of_week_text_layer, GColorClear);
  
    text_layer_set_font(day_of_week_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  //layer_add_child(window_get_root_layer(window), text_layer_get_layer(day_of_week_text_layer));
    layer_add_child(text_layer, text_layer_get_layer(day_of_week_text_layer));

  Tuplet initial_values[] = {
    TupletInteger(KEY_TEMPERATURE, temp_to_store),
    TupletCString(KEY_CONDITIONS, " "),
    TupletCString(KEY_POS, " "),
    MyTupletCString(KEY_BACKGROUND_COLOR, background_color_hex_char),
    MyTupletCString(KEY_FONT_COLOR, font_color_hex_char),
    TupletInteger(KEY_VIBRATE, vibrate_status),
    TupletInteger(KEY_UNIT, temp_units),
    TupletInteger(KEY_LOCATION, location_status),
    TupletInteger(KEY_ZIP_CODE, zip_code_int),
    TupletInteger(KEY_MIDDLE_OUTLINE, middle_outline_status),
    MyTupletCString(KEY_LATITUDE, latitude_to_store),
    MyTupletCString(KEY_LONGITUDE, longitude_to_store),
    TupletInteger(KEY_UPDATE_INTERVAL, update_interval_val),
    TupletInteger(KEY_SLEEP, sleep_status),
    TupletInteger(KEY_SLEEP_START, sleep_start_hour),
    TupletInteger(KEY_SLEEP_END, sleep_end_hour),
    TupletInteger(KEY_BATTERY_METER, battery_meter_status)
  };

  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
      initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);
  
  requestWeather();
}

static void window_unload(Window *window) 
{
  layer_destroy(canvas_layer);
  text_layer_destroy(month_text_layer);
  text_layer_destroy(day_text_layer);
  text_layer_destroy(temperature_text_layer);
  text_layer_destroy(day_of_week_text_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) 
{
  if(tick_time->tm_sec % 60 == 0) 
  {
    update_time();
  }
}

static void init(void) 
{
  if (persist_exists(KEY_TEMPERATURE))
  {
    temp_to_store = persist_read_int(KEY_TEMPERATURE);
  }

  if (persist_exists(KEY_UNIT))
  {
    temp_units = persist_read_int(KEY_UNIT);
  }
  if (persist_exists(KEY_VIBRATE))
  {
    vibrate_status = persist_read_int(KEY_VIBRATE);
  }
  if (persist_exists(KEY_LOCATION))
  {
    location_status = persist_read_int(KEY_LOCATION);
  }
  if (persist_exists(KEY_ZIP_CODE))
  {
    zip_code_int = persist_read_int(KEY_ZIP_CODE);
  }
  if (persist_exists(KEY_BACKGROUND_COLOR))
  {
    persist_read_string(KEY_BACKGROUND_COLOR, background_color_hex_char, 10);
  }
  if (persist_exists(KEY_FONT_COLOR))
  {
    persist_read_string(KEY_FONT_COLOR, font_color_hex_char, 10);
  }
  if (persist_exists(KEY_MIDDLE_OUTLINE))
  {
    middle_outline_status = persist_read_int(KEY_MIDDLE_OUTLINE);
  }
  if (persist_exists(KEY_LATITUDE))
  {
    persist_read_string(KEY_LATITUDE, latitude_to_store, 15);
  }
  if (persist_exists(KEY_LONGITUDE))
  {
    persist_read_string(KEY_LONGITUDE, longitude_to_store, 15);
  }  
  if (persist_exists(KEY_UPDATE_INTERVAL))
  {
    update_interval_val = persist_read_int(KEY_UPDATE_INTERVAL);
  }

  if (persist_exists(KEY_SLEEP))
  {
    sleep_status = persist_read_int(KEY_SLEEP);
  }
  if (persist_exists(KEY_SLEEP_START))
  {
    sleep_start_hour = persist_read_int(KEY_SLEEP_START);
  }
  if (persist_exists(KEY_SLEEP_END))
  {
    sleep_end_hour = persist_read_int(KEY_SLEEP_END);
  }
  if (persist_exists(KEY_BATTERY_METER))
  {
    battery_meter_status = persist_read_int(KEY_BATTERY_METER);
  }

  background_color_hex_int = HexStringToUInt(background_color_hex_char);
  font_color_hex_int = HexStringToUInt(font_color_hex_char);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  //bluetooth_connection_service_subscribe(BlutoothConnectionHandler);

  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  update_time();
}

static void deinit(void) 
{
  persist_write_int(KEY_TEMPERATURE, temp_to_store);
  persist_write_int(KEY_UNIT, temp_units);
  persist_write_int(KEY_VIBRATE, vibrate_status);
  persist_write_int(KEY_LOCATION, location_status);
  persist_write_int(KEY_ZIP_CODE, zip_code_int);
  persist_write_string(KEY_BACKGROUND_COLOR, background_color_hex_char);
  persist_write_string(KEY_FONT_COLOR, font_color_hex_char);
  persist_write_int(KEY_MIDDLE_OUTLINE, middle_outline_status);
  persist_write_string(KEY_LATITUDE, latitude_to_store);
  persist_write_string(KEY_LONGITUDE, longitude_to_store);
  persist_write_int(KEY_UPDATE_INTERVAL, update_interval_val);
  persist_write_int(KEY_SLEEP, sleep_status);
  persist_write_int(KEY_SLEEP_START, sleep_start_hour);
  persist_write_int(KEY_SLEEP_END, sleep_end_hour);
  persist_write_int(KEY_BATTERY_METER, battery_meter_status);

  window_destroy(window);
  tick_timer_service_unsubscribe();
  //bluetooth_connection_service_unsubscribe();

  app_sync_deinit(&s_sync);
}

int main(void) 
{
  init();
  app_event_loop();
  deinit();
}
