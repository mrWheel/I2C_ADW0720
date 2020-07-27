//
// ATMEL ATTINY841
//                                +--\/--+
//                           VCC 1|      |14 GND
//         slot0  <---  (D0) PB0 2|      |13 PA0 (D10) --> slot7
//         slot1  <---  (D1) PB1 3|      |12 PA1 (D9) ---> slot6
//          [RST] <--> RESET PB3 4|      |11 PA2 (D8) ---> slot5
//         slot2  <---  (D2) PB2 5|      |10 PA3 (D7) ---> slot4
//         slot3  <---  (D3) PA7 6|      |9  PA4 (D4) <--> [SCL] [SLK]
//   [MOSI] [SDA] <-->  (D6) PA6 7|      |8  PA5 (D5) <--> [MISO]
//                                +------+
//                       ^                        ^
//                       |                        |
//     Clockwise Pinout  +------------------------+ like Damellis core
//
//
