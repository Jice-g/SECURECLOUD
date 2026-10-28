

# Cas d’utilisation — Application cliente SECURECLOUD


## UC-01 — Se connecter au serveur

**Objectif :**
Permettre à un utilisateur authentifié d’établir une session avec le serveur SECURECLOUD.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'application cliente est installée sur le poste de l'utilisateur.
- L'utilisateur dispose d'un compte SECURECLOUD actif.
- Le serveur SECURECLOUD est accessible.

**Scénario nominal :**
1. L'utilisateur ouvre l'application cliente.
2. L'application affiche l'écran de connexion.
3. L'utilisateur saisit son identifiant et son mot de passe.
4. L'utilisateur valide la connexion.
5. Le serveur vérifie les informations d'authentification.
6. Le serveur accepte l'authentification.
7. L'application établit la session utilisateur.
8. L'application affiche l'interface principale.

**Scénarios alternatifs / exceptions :**
- Les identifiants sont incorrects → l'application indique l'échec de l'authentification et permet une nouvelle tentative.
- Le réseau est indisponible → l'application indique que le serveur est inaccessible.
- Le compte n'est pas autorisé à se connecter → l'application indique le refus de connexion.
- L'utilisateur a oublié son identifiant → il utilise la procédure de récupération prévue.
- L'utilisateur a oublié son mot de passe → il utilise la procédure de récupération/réinitialisation prévue.

**Postconditions :**
- **Succès :*- une session utilisateur authentifiée est établie.
- **Échec :*- aucune session authentifiée n'est établie.

---

## UC-02 — Envoyer un message individuel

**Objectif :**
Permettre à un utilisateur d'envoyer un message à un autre utilisateur SECURECLOUD.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur est authentifié.
- Le destinataire possède un compte SECURECLOUD actif.
- Une connexion au service SECURECLOUD est disponible.

**Scénario nominal :**
1. L'utilisateur sélectionne un destinataire.
2. L'application ouvre la conversation individuelle.
3. L'utilisateur rédige son message.
4. L'utilisateur valide l'envoi.
5. L'application transmet le message au service SECURECLOUD.
6. Le message est distribué au destinataire.
7. Le message apparaît dans le fil de conversation.

**Scénarios alternatifs / exceptions :**
- Le destinataire n'est plus disponible → le message est traité selon le mécanisme de livraison différée prévu.
- La connexion est interrompue → l'application signale l'échec ou conserve le message pour une nouvelle tentative, selon les règles définies.
- Le message ne peut pas être transmis → l'application indique son état à l'utilisateur.

**Postconditions :**
- **Succès :*- le message est accepté par le service et associé à la conversation.
- **Échec :*- l'utilisateur est informé que le message n'a pas pu être transmis.

---

## UC-03 — Envoyer un message dans un groupe

**Objectif :**
Permettre à un utilisateur d'envoyer un message à l'ensemble des membres autorisés d'un groupe de discussion.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur est authentifié.
- Le groupe existe.
- L'utilisateur est membre du groupe.
- L'utilisateur dispose du droit d'envoyer des messages dans le groupe.

**Scénario nominal :**
1. L'utilisateur sélectionne un groupe.
2. L'application ouvre la conversation du groupe.
3. L'utilisateur rédige son message.
4. L'utilisateur valide l'envoi.
5. L'application transmet le message au service SECURECLOUD.
6. Le service distribue le message aux membres autorisés du groupe.
7. Le message apparaît dans le fil de discussion du groupe.

**Scénarios alternatifs / exceptions :**
- L'utilisateur n'est plus membre du groupe → l'envoi est refusé.
- L'utilisateur ne possède pas les droits nécessaires → l'envoi est refusé.
- Un ou plusieurs membres sont momentanément indisponibles → la livraison est traitée selon le mécanisme prévu.
- La connexion est interrompue → l'application indique l'état du message et applique le mécanisme de reprise prévu.

**Postconditions :**
- **Succès :*- le message est associé au groupe et distribué aux membres autorisés.
- **Échec :*- l'utilisateur est informé du refus ou de l'échec de transmission.

---

## UC-04 — Joindre une pièce jointe à un message

**Objectif :**
Permettre à un utilisateur d'associer un fichier à un message avant son envoi.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur est authentifié.
- Une conversation individuelle ou un groupe est sélectionné.
- Le fichier respecte les règles de taille et de format définies par SECURECLOUD.

**Scénario nominal :**
1. L'utilisateur sélectionne l'option d'ajout d'une pièce jointe.
2. L'application lui permet de sélectionner un fichier.
3. L'utilisateur sélectionne le fichier.
4. L'application vérifie les contraintes applicables au fichier.
5. Le fichier est associé au message.
6. L'utilisateur valide l'envoi.
7. Le fichier et le message sont transmis selon les mécanismes de sécurité prévus.
8. Le destinataire ou les membres autorisés peuvent récupérer la pièce jointe.

**Scénarios alternatifs / exceptions :**
- Le fichier dépasse la taille maximale → l'application refuse la pièce jointe.
- Le format du fichier n'est pas autorisé → l'application refuse la pièce jointe.
- Le fichier n'est plus accessible → l'application demande à l'utilisateur de le sélectionner à nouveau.
- La transmission échoue → l'application indique l'état de la pièce jointe.

**Postconditions :**
- **Succès :*- la pièce jointe est associée au message et transmise aux destinataires autorisés.
- **Échec :*- la pièce jointe n'est pas transmise.

---

## UC-05 — Consulter ses conversations

**Objectif :**
Permettre à l'utilisateur de consulter ses conversations individuelles et de groupe.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur est authentifié.

**Scénario nominal :**
1. L'utilisateur accède à sa liste de conversations.
2. L'application affiche les conversations auxquelles il participe.
3. L'utilisateur sélectionne une conversation.
4. L'application affiche l'historique accessible de la conversation.

**Postconditions :**

- L'utilisateur peut consulter les messages auxquels il est autorisé à accéder.

---

## UC-06 — Recevoir un message

**Objectif :**
Permettre à l'utilisateur de recevoir et consulter les messages qui lui sont destinés.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur dispose d'une session active ou d'un mécanisme de réception prévu par le système.

**Scénario nominal :**
1. Le service SECURECLOUD reçoit un message destiné à l'utilisateur.
2. Le service transmet le message à l'application cliente.
3. L'application reçoit le message.
4. L'application l'intègre à la conversation correspondante.
5. L'application informe l'utilisateur de la réception.

**Postconditions :**

- Le message reçu est disponible dans la conversation concernée.

---

## UC-07 — Gérer les conversations et groupes

**Objectif :**
Permettre à l'utilisateur autorisé de créer et gérer des groupes de discussion.

**Acteur principal :**Utilisateur autorisé

**Préconditions :**
- L'utilisateur est authentifié.
- L'utilisateur dispose des droits nécessaires.

**Scénario nominal :**
1. L'utilisateur crée un groupe ou sélectionne un groupe qu'il administre.
2. L'utilisateur définit ou modifie les paramètres du groupe.
3. L'utilisateur ajoute ou retire des membres selon ses droits.
4. L'application transmet les modifications au service SECURECLOUD.
5. Le groupe est mis à jour.

**Postconditions :**

- La configuration du groupe correspond aux modifications validées.

---

## UC-08 — Télécharger / consulter une pièce jointe

**Objectif :**
Permettre à un utilisateur autorisé de récupérer une pièce jointe reçue.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur est authentifié.
- La pièce jointe appartient à une conversation à laquelle l'utilisateur a accès.

**Scénario nominal :**
1. L'utilisateur sélectionne une pièce jointe.
2. L'application vérifie que l'utilisateur est autorisé à y accéder.
3. La pièce jointe est récupérée.
4. L'application permet à l'utilisateur de la consulter ou de l'enregistrer.

**Postconditions :**
- Le fichier est accessible à l'utilisateur conformément à ses droits.

---

## UC-09 — Se déconnecter

**Objectif :**
Permettre à l'utilisateur de terminer sa session SECURECLOUD.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur possède une session active.

**Scénario nominal :**
1. L'utilisateur demande la déconnexion.
2. L'application termine la session.
3. L'application revient à l'écran de connexion.

**Postconditions :**
- La session utilisateur n'est plus active.
- Les fonctionnalités nécessitant une authentification ne sont plus accessibles.

---


LES ALERTES :
Fonctionnellement, une alerte possède une logique propre : destinataires, niveau de priorité, diffusion, accusé de réception et éventuellement suivi de l’état.
Je déconseille de mélanger « groupe de discussion » et « groupe d’alerte ». Un groupe de discussion sert à échanger ; un groupe d’alerte définit une population à laquelle une alerte critique peut être diffusée.*
Je retiendrais donc la notion de **groupe d'alerte**.


## UC-10 — Gérer les alertes prioritaires

**Objectif :**
Permettre à un utilisateur autorisé de créer, diffuser et suivre des alertes prioritaires destinées à un ensemble défini d'utilisateurs.

**Acteurs principaux :**
 - Utilisateur autorisé à émettre des alertes

**Préconditions :**
 - L'émetteur est authentifié.
 - L'émetteur dispose du droit d'émettre des alertes.
 - Les destinataires ou le groupe d'alerte existent.
 - Le service SECURECLOUD est disponible.

**Scénario nominal :**
1. L'émetteur sélectionne la fonction « Nouvelle alerte ».
2. Il définit le niveau de priorité de l'alerte.
3. Il saisit le contenu de l'alerte ou utilise un catalogue pré-défini.
4. Il sélectionne la cible de diffusion (si elle n'est pas pré-définie) :
   * un groupe d'alerte ;
   * éventuellement l'ensemble des utilisateurs autorisés.
5. L'émetteur valide la diffusion.
6. SECURECLOUD distribue l'alerte aux destinataires concernés.

**Scénarios alternatifs / exceptions :**
 - L'émetteur ne possède pas les droits nécessaires → il ne peut sélectionner cette option.
 - Le groupe d'alerte n'existe plus ou n'est pas accessible → la diffusion est refusée.
 - Certains destinataires sont momentanément indisponibles → l'alerte est conservée pour une distribution ultérieure selon les règles définies.
 - La transmission échoue → l'émetteur est informé de l'état de diffusion.

**Postconditions :**
 - **Succès :** l'alerte est enregistrée et transmise aux destinataires concernés.
 -  **Échec :** l'émetteur est informé que l'alerte n'a pas été diffusée conformément à sa cible.


## UC-11 — Recevoir une alerte

**Objectif :**
Permettre à un utilisateur du groupe d'alerte de recevoir une alerte et de l'acquitter.

**Acteurs principaux :**
 - Utilisateur du groupe d'alertes

**Préconditions :**
 - L'utilisateur est connecté
 - Le groupe d'alerte existent.
 - Le service SECURECLOUD est disponible.

**Scénario nominal :**
1. L'utilisateur reçois une alerte
2. L'alerte est visible et accessible immédiatement
3. L'utilisateur accède à l'alerte
4. L'utilisateur accède à l'alerte et acquitte
5. SECURECLOUD enregistre les acquittements

**Scénarios alternatifs / exceptions :**
 - L'utilisateur n'a pas lu l'alerte : alarme au bout d'un temps donné
 - L'utilisateur n'est pas connecté au moment de l'envoi de l'alerte : elle s'affiche dés la connexion.

**Postconditions :**
 - **Lu et acquitté :** l'alerte est notifié comme acquittement par l'utilisateur
 - **Non délivrée :** l'alerte sera notifiée dés que les conditions de connexion le permettent



# A voir pour plus tard :

## UC-11 — Gérer les groupes d'alerte ??? (V2?)

Je créerais également un cas distinct, car la gestion de la population destinataire est une fonction métier à part entière.

**Objectif :**
Permettre à un utilisateur autorisé de créer et administrer des groupes d'alerte.

**Acteur principal :**
Administrateur ou utilisateur disposant des droits appropriés.

**Préconditions :**
* L'utilisateur est authentifié.
* L'utilisateur possède les droits d'administration des groupes d'alerte.

**Scénario nominal :**

1. L'utilisateur crée un groupe d'alerte.
2. Il définit son nom et ses paramètres.
3. Il ajoute ou retire des membres.
4. Il valide la configuration.
5. Le système enregistre le groupe.

**Exemple :**

```text
Groupe d'alerte : Mission Palestine

Membres :
 ├── Utilisateur A
 ├── Utilisateur B
 ├── Utilisateur C
 └── Utilisateur D

Alertes reçues :
 ├── Évacuation
 ├── Danger imminent
 ├── Arrestation
 └── Perte de contact
```

### Un point architectural important

Je distinguerais clairement trois concepts :

```text
Utilisateur
    │
    ├──────────────► Conversation 1-to-1
    │
    ├──────────────► Groupe de discussion
    │
    └──────────────► Groupe d'alerte
                         │
                         ├── Utilisateur A
                         ├── Utilisateur B
                         └── Utilisateur C
```

Un utilisateur peut donc appartenir à plusieurs groupes de discussion **et** plusieurs groupes d'alerte.

Une alerte n'est pas nécessairement une conversation. Elle doit avoir son propre cycle de vie :

```text
Création
   ↓
Validation
   ↓
Diffusion
   ↓
Réception
   ↓
Prise en compte
   ↓
Clôture
```

Cela permet notamment de distinguer :

* **alerte envoyée**
* **alerte reçue**
* **alerte lue**
* **alerte acquittée**
* **alerte non acquittée**
* **alerte clôturée**

Pour un système destiné à des équipes terrain, l'**acquittement** est particulièrement intéressant. Par exemple, une alerte « ÉVACUATION IMMÉDIATE » pourrait exiger une confirmation de chaque membre du groupe. L'émetteur pourrait alors voir :

```text
ÉVACUATION IMMÉDIATE
Groupe : Mission Palestine

✓ Membre A — acquitté
✓ Membre B — acquitté
⚠ Membre C — non acquitté
✓ Membre D — acquitté
```

Pour formaliser la gestion des alertes

* Construire le diagramme UML des alertes
* Rédiger les exigences fonctionnelles des alertes


## Remarque importante sur le niveau d'abstraction

« joindre une pièce jointe à un message » peut être modélisé comme une extension de **Envoyer un message**, plutôt que comme une fonction totalement indépendante :

```text
Envoyer un message
       │
       ├── <<include>> Authentification
       │
       └── <<extend>> Joindre une pièce jointe
```

Il faut aussi prévoir : "télécharger une pièce jointe"

Pour le projet SECURECLOUD, je partirais donc sur un catalogue fonctionnel d'environ **8 à 12 cas d'utilisation majeurs**, puis je ferais une matrice :

**Acteurs → Cas d'utilisation → Exigences fonctionnelles → exigences de sécurité → composants techniques.**

C'est beaucoup plus propre pour le dossier RNCP et permettra ensuite de dériver directement le diagramme UML des cas d'utilisation. Le codage ne sera plus qu'une étape de "mise en code".

Je vais structurer `UseCases.md` comme un document de spécification fonctionnelle : périmètre, acteurs, conventions, puis catalogue exhaustif des cas d’utilisation avec préconditions, scénario nominal, alternatives/exceptions et postconditions.

Pour finaliser UseCases.md, il faut le confronter aux exigences fonctionnelles.
	

