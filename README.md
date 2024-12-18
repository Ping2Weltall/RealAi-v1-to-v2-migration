# RealAi-v1-to-v2-migration
Migration of a Oblivonburn RealAi v1 into v2 Sql Tables for import.

I do not even know if it makes sense but worth a try - imo.

Now - the main problem is to get your RealAI brain anywhere where it is accessible.

I splited it into several parts. 

So far 3 C program which should run out of the Box in Cxxdroid:
migration_v1_Words_v2.c
migration_v1_PreWords_v2.c
migration_v1_ProWords_v2.c

and one to transfer the Data into a previous exported RealAIv2 Brain for import or merge.
RuteShi2Brain.py

So far I do not know if it makes sense .. 

You can run it on cxxdraid and pydroid on Android.

For gnu you may need to change the strlcpy to strncpy in the C sources. Mind the '\0'.

#Computeralex
