



 
 
 # Client SecureCloud Qt
 ```
SecureCloudClient 
│
├── VisualUI
├── Dispatcher
├── FrameMaker
├── FrameReader
├── Send-Recieve
└── Persistence

```
## 1. User Interface (UI) : 
 - Permet à l'utilisateur d'accèder au menu des fonctionnalités, de rédiger des messages et d'accéder stokées sur le téléphone.
 - Les fonctionnalité :
    * Se connecter/ se déconnecter
    * envoyer des messages (avec ou sans pièce jointe) à un utilisateur (1-to-1) ou un groupe (1-to-n)
	* lire des messages
	* lancer des alertes
	* accéder à ses contacts
	* accéder à l'historique
	* accède aux données du téléphone en fonction des authorisation de l'application

## 2. FonctionEngin

   ### DispatchMngr
    * Reçoit les event de l'UI et transmet à :
	  - Persistence
	  et/ou
	  - FrameMaker
	* Reçaoit les event de FrameReader et trasment à :
	  - UI
	  - Persistence	
	  - Etat connexion au service

	### FrameMaker
	 - Fabrique les trames selon les fonctionnalités activé par l'utilisateur en y insérant les données demandées (texte, média)
	 - Crypte la trame
	 - Ajoute le token network.
	 - Transmet à SendReceiveMngr
	 
	### FrameReader
	 - Décrypte les trames et les lit
	 - Stocke les données daans la base du téléphone
	 - Envoie un event à l'UI
	 
	### SendeReceiverMngr
	 - Envoi les trames avec l'URL http://... par le WebSocket (send)
	 - Reçois les messages par WebSocket (listen)
	  
	## Persistence
	  - Stoke l'activité de SecureCloudClient (la DB est cryptée).
 


```
Qt
│
├── UI
├── frameMaker
├── send-recieve
├── persist
│
└── Campaigns
      │
      ├── Actors
      ├── Items
      ├── Locations
      └── Factions
```



