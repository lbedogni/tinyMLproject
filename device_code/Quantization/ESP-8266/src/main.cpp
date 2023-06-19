/*
Uso libreria TensorFlowLite_ESP32 allo stesso modo spiegato per ESP-32.
Per poterla adattare all'ESP-8266 però ho dovuto eliminare alcune directory (es. esp_nn, ecc.).
*/

#include <Arduino.h>

#include <TensorFlowLite_ESP32.h>

#include "RoomOccupancyModel_large_quant.h"

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

int curr_prediction = 0;
int total_prediction = 200;

const int kTensorArenaSize = 5 * 1024;    // 2 per RoomOccupancy, 5 per DrowsinessDetetction, 3 per TimeSeries

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

  Serial.println("ESP-8266, Quantizzazione");

}

void loop() {

  if(curr_prediction == total_prediction){
    return;
  }

  uint32_t start = micros();

  // set input
  for (int i=0; i<input_size; i++){
    input->data.int8[i] = x[i] / input->params.scale + input->params.zero_point;
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
    y[i] = (output->data.int8[i] - output->params.zero_point) * output->params.scale;
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