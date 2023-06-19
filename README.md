# TinyML Performance Evaluation #

Repository del progetto "TinyML Performance Evaluation" di IoT.
E' composta nel seguente modo:
* **file TinyMLperformanceEval.pdf**: report completo del progetto;
* **folder python**: contiene i notebook e i dataset usati per la creazione delle reti neurali ed anche il notebook per l'elaborazione dei risultati come grafici;
* **folder device_code**: contiene una cartella per ogni tipo di problema trattato (room occupancy detection, drowsiness detection, time series e la quantizzazione). Per ciascuna è riportato il codice relativo a ogni device (ESP-32, Arduino Nano 33 BLE Sense, ESP-8266). Per ESP-32 e ESP-8266 è stato usato PlatformIO, mentre per Arduino Nano 33 BLE Sense è usato Arduino IDE. Per poter eseguire Arduino è necessario scaricare le opportune librerie (Arduino_TensorFlowLite e EloquentTinyML) che non sono presenti in questo repository.