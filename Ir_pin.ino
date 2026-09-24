int ir_pin = 6;
int led_pin =13;
int hasObstacle =LOW;

void setup() {
  pinMode(ir_pin,INPUT);
  pinMode(led_pin,OUTPUT);
  Serial.begin(9600);


}

void loop() {
hasObstacle = digitalRead(ir_pin);

if(hasObstacle ==LOW){
  Serial.println("Stop");
  digitalWrite(led_pin,LOW);

}
else{
  Serial.println("Go ahead Path is Clear");
  digitalWrite(led_pin,HIGH);
  
}
delay(500);
}
