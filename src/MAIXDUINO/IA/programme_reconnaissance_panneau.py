
#========================== Programme Base TP2 ==== HD 2024 ======================================

import sensor, image, lcd
import KPU as kpu

from fpioa_manager import fm # fpioa_manager: abbreviated as fm
from Maix import GPIO

#fm.register(14, fm.fpioa.GPIO2) # Attribue la pin IO12 (Led RGB Verte) à la fonction logicielle GPIO0
fm.register(12, fm.fpioa.GPIO0) # Attribue la pin IO12 à la fonction logicielle GPIO0
fm.register(13, fm.fpioa.GPIO1) # IO12 = Led RGB Verte / IO13 = Led RGB Rouge
fm.register(14, fm.fpioa.GPIO2)

led_verte = GPIO(GPIO.GPIO0, GPIO.OUT) # Création de l'objet "led_verte" de classe GPIOe
led_rouge = GPIO(GPIO.GPIO1, GPIO.OUT) # Dans les 2 cas, led est un GPIO de type OUT
led_bleue = GPIO(GPIO.GPIO2, GPIO.OUT)

#---------- Initialisation LCD et Caméra -----------
lcd.init()
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.run(1)
sensor.set_windowing((224,224))


#---------- Initialisation KPU avec le modèle IA entrainé au préalable -------------------------

modele = kpu.load(0x300000)   # Chargement des coefficients du modèle depuis l'adresse 0x300000

anchors = (1.55,2.12,5.42,5.48,0.66,0.91,3.66,4.0,6.75,6.66)

kpu.init_yolo2(modele, 0.5, 0.3, 5, anchors) # Initialisation du calculateur IA

#-------- Boucle principale : Capture image -> Calcul IA -> Affichage résultats ---------------

while True:
        img = sensor.snapshot()  # Capture d'une image
        Resultats = kpu.run_yolo2(modele, img)  # Récupération du résultat du calculateur IA
        if Resultats :
            for i in Resultats :  # il peut y avoir plusieurs identifications
                img.draw_rectangle(i.rect(),color=(0,255,0)) # Tracé de la boite englobante
                if (i.classid()==0):
                    objet = "panneau 130"
                    led_bleue.value(0) # Led en cyan
                    led_verte.value(0)
                if (i.classid()==1):
                    objet = "panneau sens interdit"
                    led_rouge.value(0) # Led en blanc
                    led_bleue.value(0)
                    led_verte.value(0)
                if (i.classid()==2):
                    objet = "feu rouge"
                    led_rouge.value(0) # Led en rouge
                if (i.classid()==3):
                    objet = "panneau stop"
                    led_rouge.value(0) # Led en jaune
                    led_verte.value(0)
                if (i.classid()==4):
                    objet = "panneau 50"
                    led_rouge.value(0) # Led en violet
                    led_bleue.value(0)
                print(objet)
        else:
            led_bleue.value(1)
            led_rouge.value(1)
            led_verte.value(1)

        lcd.display(img)

