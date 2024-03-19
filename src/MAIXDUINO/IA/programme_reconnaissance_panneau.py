#========================== Programme Base TP2 ==== HD 2024 ======================================

import sensor, image, lcd
import KPU as kpu
import time

from fpioa_manager import fm # fpioa_manager: abbreviated as fm
from Maix import GPIO

fm.register(14, fm.fpioa.GPIO0) # Attribue la pin IO12 (Led RGB Verte) à la fonction logicielle GPIO0
led = GPIO(GPIO.GPIO0, GPIO.OUT) # Création de l'objet "led" de classe GPIO en sortie

#---------- Initialisation LCD et Caméra -----------
lcd.init()
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.run(1)
sensor.set_windowing((224,224))

start_time = time.ticks_ms()
frame_count = 0

#---------- Initialisation KPU avec le modèle IA entrainé au préalable -------------------------

modele = kpu.load(0x300000)   # Chargement des coefficients du modèle depuis l'adresse 0x300000

#anchors = (3.59,  3.16, 1.5, 0.73, 4.69, 5.25, 6.28, 3.44, 1.97, 1.66)
anchors = (1.56,  1.39, 3.59, 3.34, 3.17, 1.89, 5.14, 5.42, 6.22, 3.44)

kpu.init_yolo2(modele, 0.5, 0.3, 5, anchors) # Initialisation du calculateur IA

#-------- Boucle principale : Capture image -> Calcul IA -> Affichage résultats ---------------

while True:
        img = sensor.snapshot()  # Capture d'une image
        Resultats = kpu.run_yolo2(modele, img)  # Récupération du résultat du calculateur IA
        if Resultats :
            led.value(0)
            for i in Resultats :  # il peut y avoir plusieurs identifications
                img.draw_rectangle(i.rect(),color=(0,255,0)) # Tracé de la boite englobante
                print("Panneau trouvé !")    # Sur terminal série, affichage du contenu de chaque résultat
        else:
            led.value(1)
        frame_count +=1
        curent_time = time.ticks_ms()

        fps = frame_count / ((curent_time - start_time ) / 1000 )
        img.draw_string(10,0,str(int(fps)),lcd.GREEN,scale =5)
        frame_count = 0
        start_time = curent_time

        lcd.display(img)

