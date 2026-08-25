# Fritzing parts

## Descrizione
I componenti di fritzing sono salvati all'interno della cartella `fritzing-...2.app/Contents/MacOS/fritzing-parts` e sono mantenuti aggiornati con il branch `master` della seguente repo di github: [https://github.com/fritzing/fritzing-parts.git](https://github.com/fritzing/fritzing-parts.git). Per archiviare correttamente tutti i files dell'applicazione, è stato scelto di disabilitare ed eliminare il sistema di tracciamento git dalla cartella in questione. Potrebbe essere necessario in futuro riabilitare il sistema di tracciamento git e ristabilire il collegamento con il corrispettivo branch della repo di github per aggiornare i componenti della libreria.

## Ripristino del collegamento
Per ripristinare il collegamento, conviene eliminare la cartella `fritzing-parts` e clonare nuovamente la repo di github al suo posto con i seguenti comandi:
```sh
# naviga fino alla cartella dove installare i componenti di fritzing
cd .../fritzing-...2.app/Contents/MacOS

# elimina la cartella esistente dei componenti di fritzing
rm -r fritzing-parts

# clona il branch master della repo di github dei componenti di fritzing
git clone --branch master https://github.com/fritzing/fritzing-parts.git
```

## Stato commit tree della repo archiviata
```
% git remote -v
origin	https://github.com/fritzing/fritzing-parts.git (fetch)
origin	https://github.com/fritzing/fritzing-parts.git (push)

% git log -1 --format=fuller
commit e64ffe973e92176b989ab390ab668638a85ee305 (HEAD -> master, origin/master)
Author:     Kjell <kjell@gmx.de>
AuthorDate: Fri Mar 10 07:23:36 2023 +0100
Commit:     GitHub <noreply@github.com>
CommitDate: Fri Mar 10 07:23:36 2023 +0100

    Update CONTRIBUTING.md

    Added and updated hints for submitting pull requests
```
