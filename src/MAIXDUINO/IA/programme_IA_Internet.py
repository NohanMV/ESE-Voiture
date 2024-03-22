
#========================== Programme Base TP2 ==== HD 2024 ======================================

import sensor, image, lcd
import KPU as kpu
import socket , network
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
class wifi():

    nic = None

    def reset(force=False, reply=5, is_hard=True):
        if force == False and __class__.isconnected():
            return True
        try:
            # IO map for ESP32 on Maixduino
            fm.register(25,fm.fpioa.GPIOHS10)#cs
            fm.register(8,fm.fpioa.GPIOHS11)#rst
            fm.register(9,fm.fpioa.GPIOHS12)#rdy

            if is_hard:
                print("Use Hareware SPI for other maixduino")
                fm.register(28,fm.fpioa.SPI1_D0, force=True)#mosi
                fm.register(26,fm.fpioa.SPI1_D1, force=True)#miso
                fm.register(27,fm.fpioa.SPI1_SCLK, force=True)#sclk
                __class__.nic = network.ESP32_SPI(cs=fm.fpioa.GPIOHS10, rst=fm.fpioa.GPIOHS11, rdy=fm.fpioa.GPIOHS12, spi=1)
                print("ESP32_SPI firmware version:", __class__.nic.version())
            else:
                # Running within 3 seconds of power-up can cause an SD load error
                print("Use Software SPI for other hardware")
                fm.register(28,fm.fpioa.GPIOHS13, force=True)#mosi
                fm.register(26,fm.fpioa.GPIOHS14, force=True)#miso
                fm.register(27,fm.fpioa.GPIOHS15, force=True)#sclk
                __class__.nic = network.ESP32_SPI(cs=fm.fpioa.GPIOHS10,rst=fm.fpioa.GPIOHS11,rdy=fm.fpioa.GPIOHS12, mosi=fm.fpioa.GPIOHS13,miso=fm.fpioa.GPIOHS14,sclk=fm.fpioa.GPIOHS15)
                print("ESP32_SPI firmware version:", __class__.nic.version())

            # time.sleep_ms(500) # wait at ready to connect
        except Exception as e:
            print(e)
            return False
        return True

    def connect(ssid="wifi_name", pasw="pass_word"):
        if __class__.nic != None:
            return __class__.nic.connect(ssid, pasw)

    def ifconfig(): # should check ip != 0.0.0.0
        if __class__.nic != None:
            return __class__.nic.ifconfig()

    def isconnected():
        if __class__.nic != None:
            return __class__.nic.isconnected()
        return False

if __name__ == "__main__":
    # It is recommended to callas a class library (upload network_espat.py)

    # from network_esp32 import wifi
    SSID = "ESP32_AP"
    PASW = "mot_de_passe"
    ESP32_IP = "192.168.4.1"
    ESP32_PORT =8080

    def check_wifi_net(reply=5):
        if wifi.isconnected() != True:
            for i in range(reply):
                try:
                    wifi.reset(is_hard=True)
                    print('try esp32spi connect wifi...')
                    wifi.connect(SSID, PASW)
                    if wifi.isconnected():
                        break
                except Exception as e:
                    print(e)
        return wifi.isconnected()

    if wifi.isconnected() == False:
        check_wifi_net()
    print('network state:', wifi.isconnected(), wifi.ifconfig())

    sock = socket.socket()
    sock.connect((ESP32_IP,ESP32_PORT))

    while True:
            img = sensor.snapshot()  # Capture d'une image
            Resultats = kpu.run_yolo2(modele, img)  # Récupération du résultat du calculateur IA
            if Resultats :
                for i in Resultats :  # il peut y avoir plusieurs identifications
                    img.draw_rectangle(i.rect(),color=(0,255,0)) # Tracé de la boite englobante
                    if (i.classid()==0):
                        objet = " panneau 130"
                        led_bleue.value(0) # Led en cyan
                        led_verte.value(0)
                        img.draw_string(0, 0, " panneau 130 ", color=(0x00,0xFF,0xFF), scale=1)
                    if (i.classid()==1):
                        objet = " panneau sens interdit"
                        led_rouge.value(0) # Led en blanc
                        led_bleue.value(0)
                        led_verte.value(0)
                        img.draw_string(0, 0, " panneau sens interdit ", color=(0xFF,0xFF,0xFF), scale=1)
                    if (i.classid()==2):
                        objet = "feu rouge"
                        led_rouge.value(0) # Led en rouge
                        img.draw_string(0, 0, " feu rouge ", color=(0xFF,0x00,0x00), scale=1)
                    if (i.classid()==3):
                        objet = "panneau stop"
                        led_rouge.value(0) # Led en jaune
                        led_verte.value(0)
                        img.draw_string(0, 0, " panneau stop ", color=(0xFF,0xFF,0x00), scale=1)
                    if (i.classid()==4):
                        objet = "panneau 50"
                        led_rouge.value(0) # Led en violet
                        led_bleue.value(0)
                        img.draw_string(0, 0, " panneau 50 ", color=(0xFF,0x00,0xFF), scale=1)
                    print(objet)
                    data_to_send =objet
                    sock.send(data_to_send.encode())

            else:
                led_bleue.value(1)
                led_rouge.value(1)
                led_verte.value(1)

            lcd.display(img)
