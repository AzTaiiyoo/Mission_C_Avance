# README

## Lancement du robot

Notre projet est fonctionnel sur le **simulateur 2D**.

Pour le lancer voici les actions à réaliser :

1. Lancez la simulation :
   ```bash
   ./simulateur
   ```

## Machine à État

La gestion de l'état du robot est modélisée à l'aide d'une **machine à état**. La machine à état définit les différents états dans lesquels l'ampoule peut se trouver (allumée, éteinte, etc.) et les transitions possibles entre ces états. Vous pouvez consulter le diagramme de la machine à état dans le fichier **MaE.png**.

![Machine à état](mae.png)

## Découpage du Programme

Le programme est découpé en plusieurs modules distincts pour assurer une gestion claire et modulaire des différents éléments :

1. **Module Lampe** : Ce module gère l'état de la lampe, notamment son allumage et son extinction. Il utilise la machine à état pour déterminer les transitions.
2. **Module Ampoule** : Ce module contrôle l'état de l'ampoule (allumée, éteinte). Il réagit aux commandes envoyées par le module Lampe.

3. **Module Principal** : Le module principal initialise les modules Lampe et Ampoule, et assure la communication entre eux. Il orchestre l'exécution du programme et gère les entrées utilisateurs.

4. **Simulation** : Ce module permet d'exécuter la simulation dans l'environnement 2D et d'afficher l'état actuel de la lampe et de l'ampoule sur l'interface graphique.

## Conclusion

Ce projet implémente une gestion d'état pour un système simple de lampe et ampoule, en utilisant une machine à état pour modéliser les transitions. L'utilisation d'un simulateur 2D permet de visualiser facilement les interactions et de tester différents scénarios.
