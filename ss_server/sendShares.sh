#!/bin/bash

mail=0
emails_f=`cat emails` 

# Preparamos la lista de correos
IFS=','
emails=($emails_f)

for i in shares/*; do
	echo "Su compartición está adjunta a este correo." > mutt -a $i -s "Secreto compartido" ${emails[$mail]}
	mail=$[mail+1]
done
