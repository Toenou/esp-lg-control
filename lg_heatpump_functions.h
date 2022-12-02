    static const char* state_string_friendly[] = {"Initialiseren","Uit","Start","Opstarten","Aan (stabiliseren)","Aan (verwarmen)","Aan (overshoot)","Aan (stall)","Pauze (Uit)","Aan (Warm Water)","Ontdooien","Uitschakelen","Nadraaien"};
    static const char* state_string[] = {"INIT","IDLE","START","STARTING","STABILIZE","RUN","OVERSHOOT","STALL","WAIT","SWW","DEFROST","HALT","AFTERRUN"};
    enum States {INIT,IDLE,START,STARTING,STABILIZE,RUN,OVERSHOOT,STALL,WAIT,SWW,DEFROST,HALT,AFTERRUN};

    //update target temp through modbus
    void set_target_temp(float target){
      auto water_temp_call = id(water_temp_target_output).make_call();
      water_temp_call.set_value(round(target));
      water_temp_call.perform();
      ESP_LOGD("set_target_temp", "Modbus target set to: %f", round(target));
    }
    
    void calculate_derivative(float tracking_value,std::vector<float>* derivative, float* derivative_D_5,float* derivative_D_10){
      //calculate derivative
      float d_number = tracking_value;
      derivative->push_back(d_number);
      //limit size to 31 elements(15 minutes)
      if(derivative->size() > 31) derivative->erase(derivative->begin());
      //calculate current derivative for 5 and 10 minutes
      //derivative is measured in degrees/minute
      *derivative_D_5 = 0;
      *derivative_D_10 = 0;
      //wait until derivative > 14, this makes sure the first 2 minutes are skipped
      //first minute or so is unreliabel if pump has been off for a while (water cools in the unit)
      if(derivative->size() > 14){   
        *derivative_D_5 = (derivative->back()-derivative->at(derivative->size()-11))/10;
      }
      if(derivative->size() > 24){   
      *derivative_D_10 = (derivative->back()-derivative->at(derivative->size()-21))/20;
      id(derivative_value).publish_state(*derivative_D_10*60);
     }
    }
  
    bool check_switch_off(States state, uint32_t run_time, uint32_t run_start_time){
      if(id(thermostat_signal).state) return false; //thermostat still requesting heat
      //thermostat switched off
      //if not actually running, go straight to halt
      if(!id(compressor_running).state) return true;
      //Else check minimum run time has passed
      if((run_time - run_start_time) > (id(minimum_run_time).state * 60)) return true;
      return false;
    }
  
    //calculate stooklijn function
    void calculate_stooklijn(){
      //Calculate stooklijn target
      //Hold previous script run oat value
      static float prev_oat = 15; //oat above minimum water temp to prevent strange events on startup
      //wait for a valid oat reading
      float oat = 15;
      if(id(buiten_temp).state > 60 || id(buiten_temp).state < -50 || isnan(id(buiten_temp).state)){
        oat = prev_oat;
        //use prev_oat (or 15) and do not set update_stooklijn to false, to trigger a new run on next cycle
        ESP_LOGD("calculate_stooklijn", "Invalid OAT (%f) waiting for next run", id(buiten_temp).state);
      }  else {
        oat = round(id(buiten_temp).state);
        prev_oat = oat;
        id(update_stooklijn) = false;
      }

      //Formula is wTemp = (Z x (stooklijn_start_temp - OAT)) + stooklijn_min_wtemp         
      //Formula to calculate Z = 0-((stooklijn_max_wtemp-stooklijn_min_wtemp)) / (stooklijn_min_oat - stooklijn_start_temp))
      const float Z =  0 - (float)((id(stooklijn_max_wtemp).state-id(stooklijn_min_wtemp).state)/(id(stooklijn_min_oat).state - id(stooklijn_start_temp).state));
      //If oat below minimum oat, clamp to minimum value
      clamp(oat,id(stooklijn_min_oat).state,id(stooklijn_start_temp).state);
      id(stooklijn_target) = (int)round( (Z * (id(stooklijn_start_temp).state-oat))+id(stooklijn_min_wtemp).state);
      //Add stooklijn offset
      id(stooklijn_target) = id(stooklijn_target) + id(wp_stooklijn_offset).state;
      //Clamp target to minimum temp/max water+3
      clamp(id(stooklijn_target),id(stooklijn_min_wtemp).state,id(stooklijn_max_wtemp).state+3);
      ESP_LOGD("calculate_stooklijn", "Stooklijn calculated with oat: %f, Z: %f, offset: %f, result: %f",oat, Z, id(wp_stooklijn_offset).state, id(stooklijn_target));
      //Publish new stooklijn value to watertemp value sensor
      id(watertemp_target).publish_state(id(stooklijn_target));
      return;
    }

