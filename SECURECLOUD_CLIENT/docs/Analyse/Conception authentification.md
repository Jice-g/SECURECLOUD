# **Périmètre fonctionnel du Use Case**


 > ***Je propose de retenir le scénario suivant pour notre MVP.***

**UC-AUTH-01** — *Authentifier un utilisateur*
---

  
## **Scénario nominal**

###  1 - Coté Client

**1. Saisie des identifiants**
   
   L'utilisateur renseigne son login et son mot de passe dans LoginWindow.  

**2. Validation locale**
   
   Le client vérifie la validité syntaxique du login et la présence du mot de passe.  
   
**3. Déclenchement du Use Case**
   
   LoginUserUseCase reçoit la demande d'authentification.  
   
**4. Transmission à API_Gateway**
   
   Le client transmet les données d'authentification par un canal TLS.  

  
## 2 - Coté serveur

**5. Routage vers auth.service**
   
   La passerelle relaie la demande au microservice d'authentification.  
   
**6. Vérification serveur**
   
   auth.service vérifie les identifiants et les règles d'authentification.  
   
**7. Retour de la réponse**
   
   La réponse revient par API_Gateway vers le client.  
   
**8. Établissement de la session**
   
   En cas de succès, le client traite les jetons reçus et initialise son état de session.  
   
**9. Ouverture de l'application**
   
   L'utilisateur accède à ChatWindow.  
   

**Les scénarios alternatifs devront également être définis (dans un second temps) :  
- Login ou mot de passe incorrect.
- Champs incomplets.
- Compte désactivé ou bloqué.
- Serveur inaccessible.
- Délai d'attente dépassé.
- Réponse serveur invalide.
- Jeton absent, expiré ou invalide.
Attention à un point de sécurité : les erreurs présentées à l'utilisateur ne doivent pas permettre de distinguer inutilement un login inexistant d'un mauvais mot de passe.
<br>

## **Architecture cible du parcours**

### UI — LoginWindow

**Collecte des identifiants et affichage des résultats **
QWidget :
 - Push_Button ouvre une fenêtre LoginWin
 - EditUserName (QTextEdit)
 - EditPassword (QTextEdit)


### Application — LoginUser_UseCase

Orchestration du scénario d'authentification

### Port — IAuthGateway / ITransport

Contrat de communication avec le serveur

### Infrastructure — Client HTTP(S)  

Implémentation technique du transport TLS

### API_Gateway

Identification : `/auth`
Routage vers micro-service.

### Microservice `auth.service`

Vérification des identifiants et émission des jetons  

<br>

## **Choix technique de développement**  

### HTTP(S) ou WebSocket ?

HTTP(S) pour la requête initiale de login. WebSocket pourra être utilisé ensuite pour les échanges temps réel, avec authentification de la connexion à partir du jeton obtenu.

C'est une séparation plus simple à concevoir, tester et sécuriser.

### Gestion des jetons
Le serveur doit être responsable de leur émission et de leur signature. Le client ne doit jamais posséder de clé privée de signature JWT. Son rôle consiste à recevoir, conserver de façon sécurisée et transmettre les jetons selon les besoins du protocole.

Il faudra notamment définir la durée de vie du jeton d'accès et le mécanisme de renouvellement.  
* Pour le premier test E2E, nous pourrons limiter le périmètre à un jeton d'accès si le contrat serveur le permet.
 
<br>

# <u>**Critères de réussite du E2E**</u>

## Le test final devra démontrer la chaîne complète :
  
**Objectif de validation :** 

| **Vérification**	| **Résultat attendu** 
|---|---|
| Saisie du login	| *Acceptée si valide*
| Saisie du mot de passe  | *Transmise sans persistance locale en clair*
| Requête client| *Transmise en TLS*
| API_Gateway| *Requête correctement routée*
| auth.service| *Identifiants vérifiés*
| Authentification valide| *Jeton reçu*
| Session client| *État authentifié*
| Interface| *Ouverture de ChatWindow*
| Identifiants incorrects| *Refus proprement géré*
| Serveur indisponible | *Erreur réseau correctement gérée*  

<br>

# <u>**Première étape : cadrage technique**</u>
Avant de modifier le code, il nous manque une information déterminante : le contrat d'interface entre API_Gateway et le client.

- L'endpoint d'authentification (par exemple POST /auth/login).
- Le format de la requête JSON.
- Le format de la réponse JSON.
- Le mécanisme d'émission du JWT.
- Les codes de retour et les erreurs.




