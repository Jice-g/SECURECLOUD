# Couverture des UseCases par le domaine
<br>
  
  
| UC | Objets domaine déjà là | Manque pour le couvrir |
|---|---|---|
| UC-01 Connexion | User, UserLogin, UserRole | Entité Session (+ statut, révocation). Le mot de passe n’est pas une entité : c’est un DTO | | d’entrée de l’Application, jamais stocké. |
| UC-02 Message 1-to-1 | Message, DirectConversation, MessageContent, Priority, repos | Règle « la conversation naît au premier message » : un | | findDirectBetween + création, coordonné par le UseCase. Idempotence/ID client (B3). |
| UC-03 Groupe | GroupConversation (déjà complet) | RM : le comportement reste hors SK, mais la directive §4.4 est respectée. |
| UC-04 Pièce jointe | AttachmentId seulement | Entité Attachment (taille, type, TransferStatus), AttachmentPolicy (limites). Point ouvert n°1 de | | Domain.md : nom de fichier en clair ou non.
| UC-05 Consulter | repos | Un ConversationSummary (dernier message, non-lus) : un DTO d’Application, pas du domaine. |
| UC-06 Réception | Message | B2 : fabrique received() / statut initial DELIVERED. |
| UC-07 Gérer groupes | GroupConversation::add/remove | RM. |
| UC-08 Télécharger PJ | IsParticipating | Attachment + contrôle d’accès via la conversation. |
| UC-09 Déconnexion | – | Session (idem UC-01). |
| UC-10/11 Alertes | Priority, UserRole | Alert, AlertGroup, Acknowledgment, AlertStatus, Permission::ALERT_EMIT, PermissionPolicy. La plus grosse | part du travail restant, et le cœur fonctionnel retenu (ADR MSG-05). |

---

