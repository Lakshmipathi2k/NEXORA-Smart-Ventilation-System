# NEXORA Smart Ventilation System

An IoT-based smart safety and ventilation system built using ESP32.

## Features

• Temperature monitoring using DHT22  
• Automatic fan speed control  
• Flame detection for fire safety  
• Automatic window opening during high temperature  
• Smart door automation using IR sensors  
• People counting system  
• Automatic light control using relay  
• Real-time web dashboard using ESP32  

## Hardware Used

- ESP32
- DHT22 Temperature Sensor
- Flame Sensor
- IR Sensors (2)
- Servo Motor (Door)
- Servo Motor (Window)
- Relay Module
- Buzzer
- PWM Fan

## How it Works

The ESP32 reads temperature and flame data.  
If temperature increases or flame is detected:

- Window opens automatically
- Fan speed increases
- Buzzer alerts

IR sensors detect entry and exit to count people.  
Lights automatically turn ON/OFF based on occupancy.

## Author

Lakshmipathi S  
Electronics & Communication Engineering Student
