#!/bin/bash

password=$1
emails_f=`cat emails`

# Preparamos la lista de correos
IFS=','
emails=($emails_f)
mail=0

unset -v IFS
for i in $(seq ${#emails[@]}); do	
	echo -e "La clave del secreto es:\t$password . Used recibirá su compartición en breve.\n" > mutt -s "Secreto compartido. Clave '${emails[$mail]}'"
	echo "Se ha enviado la clave a '${emails[$mail]}'"
	mail=$[mail+1]
done
