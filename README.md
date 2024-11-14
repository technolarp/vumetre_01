# VU-mètre
Cet objet est basé sur le kit technoLARP. Il permet d'allumer un nombre plus ou moins important de led RGB grâce à un potentiomètre ou via un navigateur internet accessible via le wifi. Le nombre de leds, les couleurs et les seuils de changement de couleur sont configurables.

- Le VU-mètre permet de simuler une jauge lumineuse grâce à des leds RGB multicolores. 
- Le potentiomètre est optionnel, il permet d’allumer plus ou moins de led. Tourner le potentiomètre vers la droite ou la gauche pour allumer et éteindre les leds.
- Les leds peuvent aussi être contrôlées via un back office accessible un navigateur internet. Pour y accéder, voir la section [Accès au webUI](#Accès au webUI)
- Il est possible de définir plusieurs seuils qui correspondent à des couleurs différentes. Par exemple, les 2 premières led s’affichent en vert, les les 2 leds suivantes en jaune et les 4 dernières leds en rouge


**[Exemples](#Exemples)**  
**[Composants](#Composants)**  
**[Branchements](#Branchements)**  
**[BackOffice](#BackOffice)**  


## Exemples
<img src="./images/technolarp_objet_vumetre.png" width="600">

## Composants
Vous aurez besoin pour monter le VU-mètre

|  | |
| :---------------- | :------: |
| Un kit technoLarp | <img src="./images/technolarp_pcb_wemos.jpg" width="200"> |
| Un potentiomètre 10 000 Ohms | <img src="./images/technolarp_potentiometre.jpg" height="200"> |
| Un ruban ou d’un anneau de led ws2812b ou neopixel | <img src="./images/technolarp_leds_ws2812b_01.jpg" height="200"> |
| Une batterie 18650 et son support | <img src="./images/technolarp_18650.jpg" width="200"> |
| Un câble micro-USB | <img src="./images/technolarp_cable_micro-usb.png" width="200"> |

## Branchements

1. Connecter le potentimètre sur le connecteur "POT" en rouge sur le schéma
1. Connecter les leds RGB sur le connecteur "NEOPIXEL" en orange sur le schéma

1. A côté du connecteur "NEOPIXEL", positionner le cavalier sur la position 3.3V ou 5V
    1. Si le kit sera alimenté par un câble USB, positionner le cavalier en 5V <img src="./images/technolarp_pcb_choix_5V.png" width="100">  
    1. Si le kit sera alimenté par la batterie, positionner le cavalier en 3.3V <img src="./images/technolarp_pcb_choix_3V.png" width="100"> 

<img src="./images/technolarp_pcb_connecteurs.png" width="800">

## Installation
Pour installer le firmware de l'objet, il faut suivre ce [tutorial](https://github.com/technolarp/technolarp.github.io/wiki/Installation-du-firmware)  



## BackOffice
Pour se connecter au back office de l'objet, il faut suivre ce [tutorial](https://github.com/technolarp/technolarp.github.io/wiki/Connexion-au-back-office-de-l'objet-via-le-wifi)  


<img src="./images/VU-metre_back-office.png">


