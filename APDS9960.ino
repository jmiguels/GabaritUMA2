void init_APDS9960()
{
  if(apds.init())
    Serial.println(F("APDS-9960 initialization complete"));
  else
    Serial.println(F("Something went wrong during APDS-9960 init!"));    

  // Start running the APDS-9960 light sensor (no interrupts)
  if ( apds.enableLightSensor(false) )
    Serial.println(F("Light sensor is now running"));
  else
    Serial.println(F("Something went wrong during light sensor init!"));
}

void GetColor() 
{
    apds.readAmbientLight(ambient_light);   
    apds.readRedLight(red_light);
    apds.readGreenLight(green_light);
    apds.readBlueLight(blue_light);

    if(ambient_light>10)
    {
      if((red_light>2*blue_light)&&(red_light>2*green_light)) Serial.print("RED ");
      if((green_light>2*blue_light)&&(green_light>2*red_light)) Serial.print("GREEN ");
      if((blue_light>2*red_light)&&(blue_light>2*green_light)) Serial.print("BLUE ");
      if((1.0*blue_light/red_light<1.2)&&(1.0*blue_light/red_light>0.7)&&(blue_light>2*green_light)) Serial.print("PURPLE ");    
      if((1.0*green_light/red_light<1.2)&&(1.0*green_light/red_light>0.7)&&(green_light>2*red_light)) Serial.print("YELLOW ");    
    }
    else
      Serial.print("OFF ");
    
    Serial.print("(");
    Serial.print(ambient_light);
    Serial.print(",");
    Serial.print(red_light);
    Serial.print(",");
    Serial.print(green_light);
    Serial.print(",");
    Serial.print(blue_light);
    Serial.println(") ");
    //Serial.print(1.0*blue_light/red_light);
    //Serial.println(" racio");
}
