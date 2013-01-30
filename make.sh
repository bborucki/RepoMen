#This is a shell script for Making each of the components in the repo.
#Must be called within RepoMen
cd lib
make
cd ../server
make
cd ../client
make
cd ..