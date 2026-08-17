# Athena

command line book wish list management system in C++. Uses sqlite3 for the database. Books can be seperated by categories or authors for sorting. Any extra functions that people can think of is appreciated.

---
## Installation 

git clone the repo and cd into the folder

### Linux/Mac

```bash
chmod +x install.sh
./install.sh
```

### Windows (powershell)
```
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
.\install.ps1
```

The install will enable global access to the app for you to run it from any folder in the terminal.

---
## Startup (Untested)

### Windows (powershell)
Open your profile (notepad $PROFILE) and add:
```
. "C:\path\to\your\project\library-startup.ps1"
```

### Linux/Mac
```bash
chmod +x library-startup.sh
```
Add line below to bashrc or zshrc:
```
source /path/to/your/project/library-startup.sh
```
---
## Commands

### Initialize database
```
athena init
```

### Add Book Listing
```
athena add <isbn> <title> <author> <price_dollars> <link> <category>
```
### Edit Listing
```
  athena edit, -ed, update <id> [flags]
                                    -i, --isbn <isbn>
                                    -t, --title <title>
                                    -a, --author <author>
                                    -p, --price <price>
                                    -l, --link <link>
                                    -c, --category <category>
```
### Remove Listing
```
 athena -rm, delete -id <id>
 athena -rm, delete -is <isbn>
 athena -rm, delete -b <-a|-is|-t> <value>
 ```
### View Listings
```
  athena -ls, list, ls [-a <author> | -c <category> | -is <isbn>]
  ```

