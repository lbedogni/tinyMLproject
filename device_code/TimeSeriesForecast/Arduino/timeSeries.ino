/*
Non è possibile utilizzare la libreria eloquentTinyML (come per gli altri progetti) perchè non è del tutto aggiornata
con la versione più recente di Tensorflow Lite Micro e quindi LSTM non funziona.

E' stata quindi utilizzata la libreria Arduino_TensorFlowLite che ho installato nel seguente modo:
  - da terminale mi posiziono nella directory 'library' in cui ci sono tutte le librerie;
  - $ git clone https://github.com/tensorflow/tflite-micro-arduino-examples Arduino_TensorFlowLite
  - $ cd Arduino_TensorFlowLite
  - $ git pull
*/

#include <TensorFlowLite.h>

#include "include/TimeSeriesLSTMmodel_medium.h"

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
  Serial.println("Connesso");

  tflite::InitializeTarget();

  // creazione modello tflite da file header creato in python
  model = tflite::GetModel(model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
    Serial.println("Get model version: ERROR");
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
    Serial.println("Allocate tensor: ERROR");
    return;
  }

  // ottengo puntatori agli input e output del modello
  input = interpreter->input(0);
  output = interpreter->output(0);

  // tengo traccia del numero di inferenze fatte
  curr_prediction = 0;

  Serial.println("Model begin : OK");
}

void loop() {

  if(curr_prediction == total_prediction){
    return;
  }

  unsigned long start = micros();

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

  unsigned long delta = micros() - start;

  if(curr_prediction == 0){
    Serial.print("Previsione: ");
    Serial.println(y[0], 5);
  }
  Serial.print(delta);
  Serial.print(",");

  curr_prediction += 1;

  interpreter->Reset();  // Serve il reset in LSTM perchè è statefull
}

