# decaf-lang

## Utilisation

Pour tester la phase 1, il suffit de se placer dans le répertoire decaf/ et de lancer le script de test de cette façon:
```bash
$ sh test/arith_lex_test.sh
```

## Documentation

https://decaf.pages.unistra.fr/decaf-lang/

## Ordre des tâches

- [x] Analyse lexicale (flex) d'une grammaire arithmétique simple (uniquement décimaux et non hexadecimaux)
  - [x] Analyse simple - `printf`
  - [x] Analyse complète avec code de retour
- [ ] Analyse sémantique (bison) d'une grammaire arithmétique simple
  - [x] Traduction dirigé par la syntaxe -> interprétation
  - [ ] Traduction dirigé par la syntaxe -> génération de l'AST
- [ ] Génération de code intermédiaire
- [ ] Génération du code machine MIPS
- [ ] Enrichessement au fur et a mesure de la grammaire (opérateur booléen -> variables -> structure de contrôle if/else -> for)


## Ressoures
- https://www.enib.fr/~harrouet/Data/Courses/Flex_Bison.pdf
- https://steemit.com/utopian-io/@drifter1/writing-a-simple-compiler-on-my-own-combine-flex-and-bison

### Ressource Bison (Parser)
- https://www.gnu.org/software/bison/manual/html_node/Type-Decl.html 
- https://www.gnu.org/software/bison/manual/html_node/Token-Decl.html
