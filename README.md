# Athena

command line book wish list management system in C++. Uses sqlite3 for the database. Books can be seperated by categories or authors for sorting. Any extra functions that people can think of is appreciated.


##Installation 

git clone the repo and cd into the folder

###Linux/Mac

```bash
chmod +x install.sh
./install.sh
```

###Windows (powershell)
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
.\install.ps1

The install will enable global access to the app for you to run it from any folder in the terminal.

##Commands

###Initialize database
```
athena init
```

###Add Book Listing
```
athena add <isbn> <title> <author> <price_dollars> <link> <category>
```
###Edit Listing
```
  athena edit, -ed, update <id> [flags]
                                    -i, --isbn <isbn>
                                    -t, --title <title>\n"
                                    -a, --author <author>\n"
                                    -p, --price <price>\n"
                                    -l, --link <link>\n"
                                    -c, --category <category>\n"
```
###Remove Listing
```
 athena -rm, delete -id <id>\n"
 athena -rm, delete -is <isbn>\n"
 athena -rm, delete -b <-a|-is|-t> <value>\n"
 ```
### View Listings
```
  athena -ls, list, ls [-a <author> | -c <category> | -is <isbn>]\n";
  ```

