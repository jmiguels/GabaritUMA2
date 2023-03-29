void CLInerface() 
{  
  String strNew;
  int n;
  
  if(CommandStrIsComplete==0)
  {
    if(Serial.available() > 0)
    { 
      Serial.setTimeout(100),
      strNew = Serial.readString();    
      if(Echo) Serial.print(strNew);
      Commandstr = Commandstr + strNew;
    
      for(n=0; n<Commandstr.length();n++) //Backspace implementation
        if((Commandstr[n]==127))
          if(n>0)
            Commandstr=Commandstr.substring(0,n-1)+Commandstr.substring(n+1,Commandstr.length());
          else
          {
            Commandstr="";
            Serial.print(">");
          }//End of backspace implementation 
    }
  
    if ((Commandstr[Commandstr.length()-1]==10)||(Commandstr[Commandstr.length()-1]==13)) //accepts CR or LF at the end of the command
    {
      for(int m=0;m<Commandstr.length()||CommandStrIsComplete==0;m++) //para limpar RC e LF extra
      {
        if((Commandstr[m]==10)||(Commandstr[m]==13))
        {
          Commandstr = Commandstr.substring(0,m);
          CommandStrIsComplete=1;
        }
      }   
      Serial.println();
    }

  }
  else
  {
    if (Commandstr.equals("")) DoNothing(); else
    if (Commandstr.equals("?")||Commandstr.equals("help")||Commandstr.equals("Help")) Help(); else
    if (Commandstr.equals("Open")) 
    {
      Open();
      Serial.println("OK");
    }else
    if (Commandstr.equals("Close")) 
    {
      if(Close()==0) 
        Serial.println("OK");
      else
        Serial.println("NOK - Emergency");
    }else
    if (Commandstr.equals("Status")) 
    {
      if(digitalRead(OpenButton)==LOW) //ALTERAR PARA HIGH SE EMERGENGY BUTTON FOR NC
        Serial.println("Emergency pressed");
      else
        Serial.println("Emergency not pressed");
    }else
    if (Commandstr.equals("GetColor")) GetColor(); else
    if (Commandstr.equals("set echo off")) Echo=false; else
    if (Commandstr.equals("set echo on")) Echo=true; else
    if (Commandstr.equals("set power off")) {digitalWrite(EnergyCut,LOW); Serial.println("OK");} else
    if (Commandstr.equals("set power on")) {digitalWrite(EnergyCut,HIGH); Serial.println("OK");} else
    if (Commandstr.substring(0,3).equals("set")) set(Commandstr); else
    if (Commandstr.equals("LastOperation")) GetLastOperation(); else
    if (Commandstr.equals("version")) GetVersion(); else
    if (Commandstr.equals("type")) GetType(); else  
    Serial.println("Command Error\n");
  
  Serial.print(">");
  Commandstr="";
  CommandStrIsComplete=0;
  }
}

void Help()
{
  String HelpString="";  
  Serial.println("This is HELP");
  Serial.println();
  Serial.println(VersionStr);
  HelpString+="Help          - Use ?, help or Help to get help\n\r";
  HelpString+="Open          - To open the Gabarit. Returns 'OK' on success\n\r";
  HelpString+="Close         - To close the Gabarit. It only closes if the emergency button is inactive\n\r";
  HelpString+="                Returns 'OK' on sucess\n\r";
  HelpString+="                Returns 'NOK - Emergency' if the emergency button is active\n\r";
  HelpString+="Status        - To get status of emergency button\n\r";  
  HelpString+="                Returns 'Emergency pressed' if button is active\n\r";  
  HelpString+="                Returns 'Emergency not pressed' if button is inactive\n\r";  
  HelpString+="LastOperation - To get the last operation\n\r";  
  HelpString+="                Returns 'Last Command was Open' if gabarit was opened by command\n\r";
  HelpString+="                or by emergency button\n\r";  
  HelpString+="                Returns 'Last Command was Close' if gabarit was closed by command\n\r";
  HelpString+="set \"servo\"   - set the servo angle (between 0 and 180 degrees).\n\r";
  HelpString+="                Example to set the servo angle to 20 degrees: set \"servo\" \"20\"\n\r";
  HelpString+="GetColor      - Return the ambient light and RGB levels\n\r";
  HelpString+="set echo off  - Configure echo of CLI off\n\r";
  HelpString+="set echo on   - Configure echo of CLI on\n\r";
  HelpString+="set power off - Set equipment power off\n\r";
  HelpString+="set power on  - Set equipment power on\n\r";
  HelpString+="version       - Returns type of device and version\n\r";  
  HelpString+="type          - Returns type of device\n\r";  
  Serial.println(HelpString);
}


/**************************************
 **************************************/
void GetLastOperation()
{
  if(LastCommandWasOpen)
    Serial.println("Last Command was Open"); 
  else
    Serial.println("Last Command was Close");   
}

/**************************************
 **************************************/
void GetVersion()
{
  Serial.println(VersionStr);
}

/**************************************
 **************************************/
void GetType()
{
  Serial.println(DeviceTypeStr);
}

void set(String InputString)
{
  String Parameter;
  String Value;
  
  Parameter=InputString.substring(5,InputString.indexOf("\"",5));
  Value=InputString.substring(5+Parameter.length()+3,InputString.indexOf("\"",5+Parameter.length()+3));

  if (Parameter.equals("servo")) servo_1.write(Value.toInt()); else 
  {  
    Serial.println("Set Command Error\n");
    return;
  }  
  Serial.println("OK");
  //Serial.print(Parameter);
  //Serial.print(" set to ");
  //Serial.println(Value);
}
