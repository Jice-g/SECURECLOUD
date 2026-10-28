
# INSTALLATION MATERIEL : CLIENT Qt


## INSTALL WINDOWS

Poste jc.gerace
Windows 10
Nom du périphérique	DESKTOP-1HM3U7I
Processeur	AMD Ryzen 5 5500U with Radeon Graphics            2.10 GHz
Mémoire RAM installée	12,0 Go (9,86 Go utilisable)
Stockage	932 GB SSD WDC WDS100T2B0C-00PXH0
Carte graphique	AMD Radeon(TM) Graphics (2 GB)
ID de périphérique	BBCBD4EF-FA8C-4529-9828-FCB7A7A5BFDF
ID de produit	00325-82091-85421-AAOEM
Type du système	Système d’exploitation 64 bits, processeur x64
Stylet et fonction tactile	La fonctionnalité d’entrée tactile ou avec un stylet n’est pas disponible sur cet écran

Installer Qt avec Qt creator, installer CMake, GCC avec, et tout ce qu'il faut
Installer MSYS2 pour avoir des environnements linux, utiliser mingw64 pour installer les dépendances que Qt ne propose pas 
utiliser un script powerhsell ou cmd pour faire le build, test de ton application (+/- lancer l'exécutable




 




Sous Linux :
tu peux installer Qt en ligne de commande, selon la distro que tu as, ça sera une version plus ou moins récente (sur ubuntu c'est un peu à la traine, tu seras coincée à Qt 6.4 alors qu'en principe Qt en est à 6.11 ou un truc du genre)
tu peux installer tes dépendances normalement
faire un script .sh pour le build, test et run de ton app Qt 