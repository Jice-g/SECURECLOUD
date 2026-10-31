# Règles de conception

Ces règles évitent les débats au milieu du code. Je propose de les noter dans docs/decisions/.

DOM-01 : le domaine n’utilise ni Qt ni QString. Il utilise std::string en UTF-8, et la conversion se fait à la frontière UI.  
DOM-02 : les repositories ne font que de la persistance locale. Ils lisent et écrivent dans SQLite. Tout ce qui part vers le serveur passe par le port ITransport de la couche Application.  
DOM-03 : les repositories sont synchrones. SQLite local est rapide. L’asynchronisme (thread worker, signaux Qt) relève de Application/Presentation, ce qui garde le domaine testable sans Qt.  
DOM-04 : les services du domaine n’appellent pas les repositories. Ils sont purs, sans état, et reçoivent leurs données en paramètres. Les use cases orchestrent : lire, appeler le service, sauvegarder. Les services deviennent ainsi triviaux à tester.  
DOM-05 : le temps et les identifiants sont injectés. TimeStamp now est un paramètre, et les Id sont générés par un port applicatif (IIdGenerator), donc pas d’horloge système dans le domaine. L’Id d’un Message doit être généré côté client, car c’est lui qui permet le dédoublonnage et la file hors-ligne.  
DOM-06 : une règle qui touche une seule entité devient une méthode de cette entité, et une règle qui en touche deux ou plus devient un service. Par exemple, Alert::acknowledge protège « acquittement définitif », tandis que AlertService::create vérifie la permission de l’User et fige les destinataires de l’AlertGroup.  
DOM-07 : les retours sont des Result<T> (dans shared/), sans exceptions dans les interfaces. Les entités sont copiables et déplaçables, et std::optional<T> sert pour « introuvable ».  
