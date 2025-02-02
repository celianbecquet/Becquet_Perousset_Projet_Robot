#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <libpic30.h>
#include "UART_Protocol.h"
#include "CB_TX1.h"
#include "main.h"
#include "IO.h"
#include "robot.h"
#include "Utilities.h"
#include "PWM.h"
#include "UART.h"



unsigned char UartCalculateChecksum(int msgFunction, int msgPayloadLength, unsigned char* msgPayload) {
    unsigned char checksum = 0;
    checksum ^= 0xFE;
    checksum ^= (unsigned char) (msgFunction >> 8);
    checksum ^= (unsigned char) (msgFunction >> 0);
    checksum ^= (unsigned char) (msgPayloadLength >> 8);
    checksum ^= (unsigned char) (msgPayloadLength >> 0);
    int i;
    for (i = 0; i < msgPayloadLength; i++) {
        checksum ^= msgPayload[i];
    }
    return checksum;
}

void UartEncodeAndSendMessage(int msgFunction, int msgPayloadLength, unsigned char* msgPayload) {

    int j;
    unsigned char trame[6 + msgPayloadLength]; //= new unsigned char[6+msgPayloadLength];

    int pos = 0;

    trame[pos++] = 0xFE;

    trame[pos++] = (unsigned char) (msgFunction >> 8);
    trame[pos++] = (unsigned char) (msgFunction >> 0);

    trame[pos++] = (unsigned char) (msgPayloadLength >> 8);
    trame[pos++] = (unsigned char) (msgPayloadLength >> 0);

    for (j = 0; j < msgPayloadLength; j++) {
        trame[pos++] = msgPayload[j];
    }

    trame[pos++] = (unsigned char) (UartCalculateChecksum(msgFunction, msgPayloadLength, msgPayload));
    SendMessage(trame, pos);
    //serialPort1.Write(trame, 0, pos);

}


int msgDecodedFunction = 0;
int msgDecodedPayloadLength = 0;
unsigned char msgDecodedPayload[128];
int msgDecodedPayloadIndex = 0;

typedef enum {
    Waiting,
    FunctionMSB,
    FunctionLSB,
    PayloadLengthMSB,
    PayloadLengthLSB,
    Payload,
    CheckSum
} StateReception;

StateReception rcvState = Waiting;

void UartDecodeMessage(unsigned char c) {
    unsigned char calculatedChecksum, receivedChecksum;
    //Fonction prenant en entree un octet et servant a reconstituer les trames    
    switch (rcvState) {

        case Waiting:
            if (c == 0xFE) {
                rcvState = FunctionMSB;
            }
            break;

        case FunctionMSB:
            msgDecodedFunction = (int) (c << 8);
            rcvState = FunctionLSB;
            break;

        case FunctionLSB:
            msgDecodedFunction += (int) (c << 0);
            rcvState = PayloadLengthMSB;
            break;

        case PayloadLengthMSB:
            msgDecodedPayloadLength = (int) (c << 8);
            rcvState = PayloadLengthLSB;
            break;

        case PayloadLengthLSB:
            msgDecodedPayloadLength += (int) (c << 0);
            msgDecodedPayloadIndex = 0;
            rcvState = Payload;
            break;

        case Payload:
            if (msgDecodedPayloadIndex < msgDecodedPayloadLength) {
                msgDecodedPayload[msgDecodedPayloadIndex] = c;
                msgDecodedPayloadIndex++;
            }
            if (msgDecodedPayloadIndex >= msgDecodedPayloadLength)
                rcvState = CheckSum;
            break;

        case CheckSum:
            receivedChecksum = c;
            calculatedChecksum = UartCalculateChecksum(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);
            if(calculatedChecksum == receivedChecksum)
            {
                UartProcessDecodedMessage(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);
            }
          
            rcvState = Waiting;
            break;
        default:
            rcvState = Waiting;
            break;

    }
}

void UartProcessDecodedMessage(int msgFunction, int msgPayloadLength, unsigned char* msgPayload)
//Fonction appelle apres le decodage pour executer l'action correspondant au message recu
{
    
            switch (msgFunction)
            {                
//                case 0x30: // IR data
//                    flagNewIrData = true;
//                    distanceTelemetreGauche = msgPayload[0];
//                    distanceTelemetreCentre = msgPayload[1];
//                    distanceTelemetreDroit = msgPayload[2];
//                    break;
//                    
//                case 0x40:
//                    robot.flagNewVitesseData = true;
//                    robot.vitesseMoteurGauche= msgPayload[0];
//                    robot.vitesseMoteurDroit = msgPayload[1];
//                   break;
//
//                case 0x80: // Text transmission
//                    robot.flagNewReceptionData = true;
//                    robot.receivedMessage = System.Text.Encoding.UTF8.GetString(msgPayload);
//                    break;
                    
                case SET_ROBOT_STATE:
                    SetRobotState(msgPayload[0]);
                    break;
            
                case SET_ROBOT_MANUAL_CONTROL:
                    SetRobotAutoControlState(msgPayload[0]);
                    break;
                    
                case SET_CONSIGNE:
                
                   robotState.vitesseLineaireConsigne = getFloat(msgPayload, 0);
                   robotState.vitesseAngulaireConsigne = getFloat(msgPayload, 4);
                   PWMSetSpeedConsignePolaire(robotState.vitesseLineaireConsigne, robotState.vitesseAngulaireConsigne);
                
                
                
                break;
                
                default: // Unknown command
                    break;
            }
     

}

void SetRobotState (unsigned char state)
{
    robotState.stateRobot = state;   
}

void SetRobotAutoControlState (unsigned char state)
{
    robotState.isInAutomaticMode = state;
    if (robotState.isInAutomaticMode == AUTO)LED_BLEUE = 1;
    else LED_BLEUE = 0;
}

