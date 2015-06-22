  #include <pebble.h>
  #include <ctype.h>

  //test change
  /*
  #define BACKGROUND_COLOR GColorBlack
  #define TIME_COLOR GColorWhite
  #define TEXT_COLOR GColorWhite
  */

char background_color_hex_char[10] = "000000"; 
char font_color_hex_char[10] = "FFFFFF";

int vibrate_status = 2;
int temp_units = 0;
int location_status = 0;
int zip_code_int = 0;
int background_color_hex_int;
int font_color_hex_int;
int temp_to_store;
int middle_outline_status = 1;

  #define KEY_TEMPERATURE 0
  #define KEY_CONDITIONS 1
  #define KEY_LOCATION 2

  //settings keys
  #define background_color 3
  #define font_color 4
  #define vibrate 5
  #define unit 6
  #define location 7
  #define zip_code 8
  #define middle_outline 9

static Window *window;
static Layer *canvas_layer;
static Layer *text_layer;
static TextLayer *month_text_layer;
static TextLayer *day_text_layer;
static TextLayer *temperature_text_layer;
static TextLayer *day_of_week_text_layer;

static char day_buffer[10];
static char date_buffer[10];
static char month_buffer[10];

static int digit_matrix[10][5][3] = {
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

  static int digit_rounded_corners[10][5][3] = {
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
        APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: Unrecognised hex character \"%c\"", thisC);
        return 0;
      }
      ++c;
    }
    return result;  
  }

  int numbers_to_print[4] = {0,0,0,0};
  int x_start[4] = {2, 83, 2, 83};
  int y_start[4] = {2, 2, 97, 97};

  static void canvas_layer_update_proc(Layer *this_layer, GContext *ctx)
  {
    int x_offset = 19;
    int y_offset = 13;

    static int corner_radius = 3;

    //graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
    background_color_hex_int = HexStringToUInt(background_color_hex_char);
    font_color_hex_int = HexStringToUInt(font_color_hex_char);

    graphics_context_set_fill_color(ctx, GColorFromHEX(background_color_hex_int));

    graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);

    graphics_context_set_fill_color(ctx, GColorFromHEX(font_color_hex_int));

    for (int x=0; x<3; x++)
    {
      for (int y=0; y<5; y++)
      {
        for (int k=0; k<4; k++)
        {
          if (digit_matrix[numbers_to_print[k]][y][x] == 1)
          {
            int x_gap = x*x;
            int y_gap = y*y;
            
            if (digit_rounded_corners[numbers_to_print[k]][y][x] == 0)
            {
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornerNone);  
            }
            if (digit_rounded_corners[numbers_to_print[k]][y][x] == 1)
            {
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornerTopLeft);  
            }
            if (digit_rounded_corners[numbers_to_print[k]][y][x] == 2)
            {
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornerTopRight);  
            }
            if (digit_rounded_corners[numbers_to_print[k]][y][x] == 3)
            {
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornerBottomRight);  
            }
            if (digit_rounded_corners[numbers_to_print[k]][y][x] == 4)
            {
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornerBottomLeft);  
            }
            if (digit_rounded_corners[numbers_to_print[k]][y][x] == 5)
            {
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornersLeft);  
            }
            if (digit_rounded_corners[numbers_to_print[k]][y][x] == 6)
            {
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornersRight);  
            }
            if (digit_rounded_corners[numbers_to_print[k]][y][x] == 7)
            {
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornersTop);  
            }
            if (digit_rounded_corners[numbers_to_print[k]][y][x] == 8)
            {
              graphics_fill_rect(ctx, GRect(x_start[k]+x_offset*x+x, y_start[k]+y_offset*y+y, x_offset, y_offset), corner_radius, GCornersBottom);  
            }
          }
        }
      }
    }
    
    BatteryChargeState charge_state = battery_state_service_peek();

    int x_length_1 = 25;
    int x_length_2;

    switch (charge_state.charge_percent)
    {
      case 0:
      x_length_1 = 2;
      break;
      case 10:
      x_length_1 = 8.4;
      break;
      case 20:
      x_length_1 = 16.8;
      break;
      case 30:
      x_length_1 = 25;
      break;
      case 40:
      x_length_1 = 8.4;
      break;
      case 50:
      x_length_1 = 16.8;
      break;
      case 60:
      x_length_1 = 25;
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
      x_length_1 = 25;
      break;
    }

    /*if (middle_outline_status == 1)
    {
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_rect(ctx, GRect(68, 0, 8, 76), 0, GCornersAll);
      graphics_fill_rect(ctx, GRect(68, 92, 8, 77), 0, GCornersAll);
      graphics_fill_rect(ctx, GRect(65, 77, 14, 14), 1, GCornersAll);  
    } */

      if (charge_state.charge_percent <= 30)
      {      
        if (middle_outline_status == 1)
        {
          graphics_context_set_fill_color(ctx, GColorBlack);
          graphics_fill_rect(ctx, GRect(68, 0, 8, 1+x_length_1), 0, GCornersAll);
          graphics_fill_rect(ctx, GRect(68, 167-x_length_1, 8, 26 + x_length_1), 0, GCornersAll);          
        }

        graphics_context_set_fill_color(ctx, GColorRed);
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

        graphics_context_set_fill_color(ctx, GColorRed);
        graphics_fill_rect(ctx, GRect(69, 0, 6, 25), 0, GCornersAll);
        graphics_fill_rect(ctx, GRect(69, 143, 6, 25), 0, GCornersAll);  
        graphics_context_set_fill_color(ctx, GColorYellow);
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

        graphics_context_set_fill_color(ctx, GColorRed);
        graphics_fill_rect(ctx, GRect(69, 0, 6, 25), 0, GCornersAll);
        graphics_fill_rect(ctx, GRect(69, 143, 6, 25), 0, GCornersAll);  
        graphics_context_set_fill_color(ctx, GColorYellow);
        graphics_fill_rect(ctx, GRect(69, 25, 6, 25), 0, GCornersAll);
        graphics_fill_rect(ctx, GRect(69, 118, 6, 25), 0, GCornersAll);  
        graphics_context_set_fill_color(ctx, GColorGreen);
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

        graphics_context_set_fill_color(ctx, GColorGreen);
        graphics_fill_rect(ctx, GRect(69, 0, 6, 75), 0, GCornersAll);  
        graphics_fill_rect(ctx, GRect(69, 93, 6, 75), 0, GCornersAll);  
      }

      if (middle_outline_status == 1)
      {
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_fill_rect(ctx, GRect(65, 77, 14, 14), 1, GCornersAll);  
      }

      if (bluetooth_connection_service_peek())
      {
        graphics_context_set_fill_color(ctx, GColorBlueMoon);
      }
      else
      {
        graphics_context_set_fill_color(ctx, GColorRed);
      }
      graphics_fill_rect(ctx, GRect(66, 78, 12, 12), 1, GCornersAll);  


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
      strftime(month_buffer, sizeof("*******"), "%B",tick_time);

      text_layer_set_text(day_text_layer, date_buffer);
      text_layer_set_text(day_of_week_text_layer, day_buffer);
      text_layer_set_text(month_text_layer, month_buffer);

      if(canvas_layer)
        layer_mark_dirty(canvas_layer);

      if(tick_time->tm_min % 60 == 0)
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

    static void window_load(Window *window) 
    {
      Layer *window_layer = window_get_root_layer(window);
      text_layer = window_get_root_layer(window);
      GRect bounds = layer_get_bounds(window_layer);

      canvas_layer = layer_create(GRect(0, 0, 144, 168));
      layer_add_child(window_layer, canvas_layer);
      layer_set_update_proc(canvas_layer, canvas_layer_update_proc);

      month_text_layer = text_layer_create(GRect(82, 67, 72, 30));
      text_layer_set_text_alignment(month_text_layer, GTextAlignmentLeft);
      text_layer_set_background_color(month_text_layer, GColorClear);
      text_layer_set_text_color(month_text_layer, GColorFromHEX(font_color_hex_int));
      text_layer_set_font(month_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    //layer_add_child(window_get_root_layer(window), text_layer_get_layer(month_text_layer));
      layer_add_child(text_layer, text_layer_get_layer(month_text_layer));

      day_text_layer = text_layer_create(GRect(76, 67, 64, 30));
      text_layer_set_text_alignment(day_text_layer, GTextAlignmentRight);
      text_layer_set_background_color(day_text_layer, GColorClear);
      text_layer_set_text_color(day_text_layer, GColorFromHEX(font_color_hex_int));
      text_layer_set_font(day_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    //layer_add_child(window_get_root_layer(window), text_layer_get_layer(day_text_layer));
      layer_add_child(text_layer, text_layer_get_layer(day_text_layer));

      temperature_text_layer = text_layer_create(GRect(3, 67, 64, 30));
      text_layer_set_text_alignment(temperature_text_layer, GTextAlignmentLeft);
      text_layer_set_background_color(temperature_text_layer, GColorClear);
      text_layer_set_text_color(temperature_text_layer, GColorFromHEX(font_color_hex_int));
      text_layer_set_font(temperature_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));

      static char temp[10];
      if (temp_to_store == 0)
      {
       snprintf(temp, sizeof(temp), "Load");
     }
     snprintf(temp, sizeof(temp), "%d°", temp_to_store);
     text_layer_set_text(temperature_text_layer, temp);
    //layer_add_child(window_get_root_layer(window), text_layer_get_layer(temperature_text_layer));
     layer_add_child(text_layer, text_layer_get_layer(temperature_text_layer));

     day_of_week_text_layer = text_layer_create(GRect(0, 67, 63, 30));
     text_layer_set_text_alignment(day_of_week_text_layer, GTextAlignmentRight);
     text_layer_set_background_color(day_of_week_text_layer, GColorClear);
     text_layer_set_text_color(day_of_week_text_layer, GColorFromHEX(font_color_hex_int));
     text_layer_set_font(day_of_week_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    //layer_add_child(window_get_root_layer(window), text_layer_get_layer(day_of_week_text_layer));
     layer_add_child(text_layer, text_layer_get_layer(day_of_week_text_layer));
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

    if(tick_time->tm_min % 30 == 0) 
    {
      DictionaryIterator *iter;
      app_message_outbox_begin(&iter);

      dict_write_uint8(iter, unit, temp_units);  
      dict_write_uint8(iter, location, location_status);  
      dict_write_uint32(iter, zip_code, zip_code_int);  

      app_message_outbox_send();
    }

  }

  static void inbox_received_callback(DictionaryIterator *iterator, void *context)
  {
    static char temperature_buffer[8];
    static char conditions_buffer[32];
    static char weather_layer_buffer[32];
    static char location_buffer[32];

    Tuple *t = dict_read_first(iterator);
    APP_LOG(APP_LOG_LEVEL_INFO, "AppMessage Received");

    while(t != NULL) {
      switch(t->key){
        case unit:
        switch((int)t->value->int32)
        {
          case 0:
          temp_units = 0;
          break;
          case 1:
          temp_units = 1;
          break;
          case 2:
          temp_units = 2;
          break;
        }
        APP_LOG(APP_LOG_LEVEL_ERROR, "Temp_units: %d", (int)t->value->int32);
        break;
        case KEY_TEMPERATURE:
        temp_to_store = (int)t->value->int32;
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°", (int)t->value->int32);
        break;
        case KEY_CONDITIONS:
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        break;
        case KEY_LOCATION:
        snprintf(location_buffer, sizeof(location_buffer), "%s", t->value->cstring);
        break;
        case background_color:
        snprintf(background_color_hex_char, sizeof(background_color_hex_char), "%s", t->value->cstring);    
        break;
        case font_color:
        snprintf(font_color_hex_char, sizeof(font_color_hex_char), "%s", t->value->cstring);    
        break;
        case vibrate:
        switch((int)t->value->int32)
        {
          case 0:
          vibrate_status = 0;
          break;
          case 1:
          vibrate_status = 1;
          break;
          case 2:
          vibrate_status = 2;
          break;
        }
        break;
        case location:
        switch((int)t->value->int32)
        {
          case 0:
          location_status = 0;
          break;
          case 1:
          location_status = 1;
          break;
        }
        break;      
        case zip_code:
        zip_code_int = (int)t->value->int32;
        break;
        case middle_outline:
        middle_outline_status = (int)t->value->int32;
        break;
        default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key%d not recognized", (int)t->key);
        break;
      }
      t = dict_read_next(iterator);
    }

    APP_LOG(APP_LOG_LEVEL_INFO, "Zip Code = %d", zip_code_int);
    APP_LOG(APP_LOG_LEVEL_INFO, "Location Status = %d", location_status);
    APP_LOG(APP_LOG_LEVEL_INFO, "Vibrate status = %d", vibrate_status);
    APP_LOG(APP_LOG_LEVEL_INFO, "background_color = %s", background_color_hex_char);
    APP_LOG(APP_LOG_LEVEL_INFO, "font_color = %s", font_color_hex_char);

    layer_mark_dirty(canvas_layer);
    layer_mark_dirty(text_layer);

    text_layer_set_font(temperature_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text(temperature_text_layer, temperature_buffer);

    background_color_hex_int = HexStringToUInt(background_color_hex_char);
    font_color_hex_int = HexStringToUInt(font_color_hex_char);

    text_layer_set_text_color(month_text_layer, GColorFromHEX(font_color_hex_int));
    text_layer_set_text_color(day_text_layer, GColorFromHEX(font_color_hex_int));
    text_layer_set_text_color(temperature_text_layer, GColorFromHEX(font_color_hex_int));
    text_layer_set_text_color(day_of_week_text_layer, GColorFromHEX(font_color_hex_int));
  }

  static void inbox_dropped_callback(AppMessageResult reason, void *context)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped");
  }

  static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed");

    psleep(100);

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, unit, temp_units);  
    dict_write_uint8(iter, location, location_status);  
    dict_write_uint32(iter, zip_code, zip_code_int);  

    app_message_outbox_send();

  }

  static void outbox_sent_callback(DictionaryIterator *iterator, void *context)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send success!");
  }

  static void init(void) 
  {  
    if (persist_exists(KEY_TEMPERATURE))
    {
      temp_to_store = persist_read_int(KEY_TEMPERATURE);
    }

    if (persist_exists(unit))
    {
      temp_units = persist_read_int(unit);
    }

    if (persist_exists(vibrate))
    {
      vibrate_status = persist_read_int(vibrate);
    }
    if (persist_exists(location))
    {
      location_status = persist_read_int(location);
    }
    if (persist_exists(zip_code))
    {
      zip_code_int = persist_read_int(zip_code);
    }
    if (persist_exists(background_color))
    {
      persist_read_string(background_color, background_color_hex_char, 10);
    }
    if (persist_exists(font_color))
    {
      persist_read_string(font_color, font_color_hex_char, 10);
    }
    if (persist_exists(middle_outline))
    {
      middle_outline_status = persist_read_int(middle_outline);
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

    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, unit, temp_units);  
    dict_write_uint8(iter, location, location_status);  
    dict_write_uint32(iter, zip_code, zip_code_int);  
    app_message_outbox_send();

    update_time();
  }

  static void deinit(void) 
  {  
    persist_write_int(KEY_TEMPERATURE, (temp_to_store));
    persist_write_int(unit, (temp_units));
    persist_write_int(vibrate, (vibrate_status));
    persist_write_int(location, (location_status));
    persist_write_int(zip_code, (zip_code_int));
    persist_write_string(background_color, (background_color_hex_char));
    persist_write_string(font_color, (font_color_hex_char));
    persist_write_int(middle_outline, (middle_outline_status));

    window_destroy(window);
    tick_timer_service_unsubscribe();
    //bluetooth_connection_service_unsubscribe();
  }

  int main(void) 
  {
    init();
    app_event_loop();
    deinit();
  }
