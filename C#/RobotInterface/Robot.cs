﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RobotInterface
{
    public class Robot
    {
        public double distanceTelemetreGauche;
        public double distanceTelemetreDroit;
        public double distanceTelemetreCentre;
        public double vitesseMoteurGauche;
        public double vitesseMoteurDroit;
        public string receivedMessage;
        public double receivedEtape;
        public double receivedInstantCourant;
        public Queue<byte> byteListReceived = new Queue<byte>();

        public bool flagNewIrData = false;
        public bool flagNewVitesseData = false;
        public bool flagNewReceptionData = false;
        public bool flagNewEtapeData = false;
        public bool flagNewPositionData = false;
        public bool flagChecksum = false;
        public bool autoControlActivated = true;


        public Robot()
        {

        }
    }
}
