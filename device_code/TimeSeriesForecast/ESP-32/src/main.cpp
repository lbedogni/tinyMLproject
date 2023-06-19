/*
Tensorflow è utilizzata tramite la libreria TensorFlowLite_ESP32 (LSTM non funziona con EloquentTinyML).
Questa però non risulta essere del tutto aggiornata. Per poterla usare con la versione di rete creata 
sono state fatte le seguenti operazioni:
  - imposto la libreria nel progetto
  - aggiorno la versione di tflite-micro:
      - entro nella folder  nomeProgetto/.pio/libdeps/esp32dev/TensorFlowLite_ESP32/scripts
      - rendo eseguibile lo script : $ chmod +x ./sync_from_tflite_micro.sh
      - eseguo lo scripit $ ./sync_from_tflite_micro.sh 
      - questo aggiornerà i file di tflite-micro (e anche tutti gli altri) ad una versione più aggiornata
        presente a questo link: https://github.com/espressif/tflite-micro-esp-examples
  - dopo aver aggiornato tflite-micro, continuo comunque ad avere problemi di compilazione: gli errori 
    sono principalmnte "file da includere non trovato". In realtà i file sono presenti ma sono in 
    locazioni diverse: li ho quindi spostati fino a che la compilazione non è andata a buon fine
  - alcune directory le ho eliminate in quanto davano errori e non servono per questo progetto.

IMPORTANTE: se non si esegue l'aggiornamento di tflite-micro ma si usa la libreria TensorFlowLite_ESP32 così 
            com'è, i nomi di alcune funzioni sono diversi perchè la versione di tflite-micro è diversa. 
            Con la nuova versione è necessario fare riferimento agli esempio nel 
            repository: https://github.com/espressif/tflite-micro-esp-examples/blob/master/examples/hello_world/main/main_functions.cc
*/

#include <Arduino.h>
#include <esp_timer.h>

#include <TensorFlowLite_ESP32.h>

#include "TimeSeriesLSTMmodel_medium.h"

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "tensorflow/lite/micro/micro_log.h"

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

int curr_prediction = 0;
int total_prediction = 200;

const int kTensorArenaSize = 6 * 1024;
// Keep aligned to 16 bytes for CMSIS
alignas(16) uint8_t tensor_arena[kTensorArenaSize];

void setup() {
  Serial.begin(9600);
  while(!Serial);

  tflite::InitializeTarget();

  // creazione modello tflite da file header creato in python
  model = tflite::GetModel(model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
    Serial.println("Model begin: ERROR");
    return;
  }

  // Aggiunge TUTTE le operazioni possibili (bisognerebbe ottimizzare) 
  static tflite::AllOpsResolver resolver;

  // costruzione dell'interprete che eseguirà il modello
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // allocazione memoria per i tensori del modello 
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    Serial.println("Model begin: ERROR");
    return;
  }

  // ottengo puntatori agli input e output del modello
  input = interpreter->input(0);
  output = interpreter->output(0);

  // tengo traccia del numero di inferenze fatte
  curr_prediction = 0;

  Serial.println("ESP-32, TimeSeriesLSTMmodel");
}

void loop() {

  if(curr_prediction == total_prediction){
    return;
  }

  uint32_t start = micros();

  // set input
  for (int i=0; i<input_size; i++){      
    input->data.f[i] = x[i];         // var 'x' è nell'header del modello con i valori già compilati in python
  }

  // Run inference
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    MicroPrintf("Invoke failed on x \n");
    return;
  }

  // set output
  float y[output_size];
  for (int i=0; i<output_size; i++){
    y[i] = output->data.f[i];        // confronta questo output con l'output in python
  }

  uint32_t delta = micros() - start;

  if(curr_prediction == 0){
    Serial.print("Previsione: ");
    Serial.println(y[0], 5);
  }
  Serial.println(delta);

  curr_prediction += 1;

  interpreter->Reset();  // Serve il reset in LSTM perchè è statefull
}
