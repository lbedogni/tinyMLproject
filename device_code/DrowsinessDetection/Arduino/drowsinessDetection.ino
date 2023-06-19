#include "EloquentTinyML.h"  // ho dovuto modificare il file di libreria "activation_utils.h" 
                             // mettendo "return a;" dopo lo switch nella funzione "ActivationValFloat"
#include "eloquent_tinyml/tensorflow.h"

#include "include/DrowsinessDetectionModel_small.h"

#define TENSOR_ARENA_SIZE 20*1024  // store model's input, output and intermediate tensors
                                  // parti da 70 * 1024 e poi diminuisci fino a che il modello non funziona più

Eloquent::TinyML::TensorFlow::TensorFlow<input_size, output_size, TENSOR_ARENA_SIZE> tf;

int curr_prediction = 0;
int total_prediction = 200;

void setup() {
  Serial.begin(9600);
  while (!Serial) ;     // https://forum.arduino.cc/t/cant-view-serial-print-from-setup/167916

  if (tf.begin(model_data)){
    Serial.print("------ Arduino Nano 33 BLE Sense, DrowsinessDetectionModel ");
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

  if(curr_prediction != 0){
    for(int i=0; i<input_size; i++){
      x[i] = random(0, 100) / 100.0;
    }
  }  

  unsigned long start = micros();

  tf.predict(x, y);       // genera una probabilità in output

  unsigned long delta = micros() - start;

  if(curr_prediction == 0){
    Serial.print("Previsione: ");
    Serial.println(y[0], 5);
  }
  Serial.print(delta);
  Serial.print(",");

  curr_prediction += 1;

}

