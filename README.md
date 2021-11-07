# decaf-lang

# Utilisation

Pour tester la phase 1, il suffit de se placer dans le répertoire decaf/ et de lancer le script de test de cette façon:
```bash
$ sh test/arith_lex_test.sh
```
## Ordre des tâches

- [x] Analyse lexicale (flex) d'une grammaire arithmétique simple (uniquement décimaux et non hexadecimaux)
  - [ ] Analyse simple - `printf`
  - [ ] Analyse complète avec code de retour
- [ ] Analyse sémantique (bison) d'une grammaire arithmétique simple
- [ ] ?
- [ ] Génération de code intermédiaire
- [ ] Génération de code MIPS
- [ ] Enrichessement au fur et a mesure de la grammaire (opérateur booléen -> variables -> structure de contrôle if/else -> for)