
#include "EloquentTinyML.h"  // ho dovuto modificare il file di libreria "activation_utils.h" 
                             // mettendo "return a;" dopo lo switch nella funzione "ActivationValFloat"

#include "RoomOccupancyModel_large.h"

#include <Arduino.h>

#define TENSOR_ARENA_SIZE 3*1024  // store model's input, output and intermediate tensors
                                  // parti da 70 * 1024 e poi diminuisci fino a che il modello non funziona più

Eloquent::TinyML::TfLite<input_size, output_size, TENSOR_ARENA_SIZE> tf;

int curr_prediction = 0;
int total_prediction = 200;

void setup() {

  Serial.begin(9600);
  Serial.println("Connesso");

  if (tf.begin(model_data)){
    Serial.print("------ ESP-32, RoomOccupancy ");
  } else{
    Serial.println("Model begin: ERROR");
  }

}

void loop() {
  if ( curr_prediction == total_prediction ){
    return;
  }

  srand(time(NULL));         
  float y[output_size];

  unsigned long start = micros();

  tf.predict(x, y);       // genera una probabilità in output

  unsigned long delta = micros() - start;

  if(curr_prediction == 0){
    Serial.print("Previsione: ");
    Serial.println(y[0], 5);
  }
  Serial.println(delta);

  curr_prediction += 1;

}