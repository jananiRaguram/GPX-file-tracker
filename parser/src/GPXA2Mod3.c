#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "GPXParser.h"
#include "GPXHelper.h"
#include "LinkedListAPI.h"
#include <string.h>
#include <stdlib.h>


//0 == false;
//1 == true;


char* trackToJSON(const Track *tr){
    char* str;

    if(tr == NULL){
        str = malloc(sizeof(char) *3);
        strcpy(str, "{}");
        return str;
    }

     char name[256];
     if(strlen(tr->name) == 0){
         strcpy(name, "None");
     }else{
         strcpy(name, tr->name);
     }

     bool isLoop = isLoopTrack(tr, 10);
     char isLoopStr[100];
     if(isLoop == 0){
         strcpy(isLoopStr, "false");
     }else{
         strcpy(isLoopStr, "true");
     }

     ListIterator iter = createIterator(tr->segments);
     int numVal = 0;
     for(TrackSegment* seg = nextElement(&iter); seg!=NULL; seg = nextElement(&iter)){
         numVal += getLength(seg->waypoints);
     }

     float trackLen = getTrackLen(tr);
     trackLen = round10(trackLen);

     char tmp[1024];
     sprintf(tmp, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", name, numVal, trackLen, isLoopStr);

     str = malloc(sizeof(char)*strlen(tmp)+1);
     strcpy(str, tmp);

     return str;

}

char* routeToJSON(const Route *rt){
    char* str;

    if(rt == NULL){
        str = malloc(sizeof(char) *3);
        strcpy(str, "{}");
        return str;
    }

    char name[256];
    if(strlen(rt->name) == 0){
        strcpy(name, "None");
    }else{
        strcpy(name, rt->name);
    }

    bool isLoop = isLoopRoute(rt, 10);
    char isLoopStr[100];
    if(isLoop == 0){
        strcpy(isLoopStr, "false");
    }else{
        strcpy(isLoopStr, "true");
    }

    float routeLen = getRouteLen(rt);
    routeLen = round10(routeLen);

    int numVal = getLength(rt->waypoints);
    char tmp[1024];

    sprintf(tmp, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", name, numVal, routeLen, isLoopStr);

    str = malloc(sizeof(char)*strlen(tmp)+1);
    strcpy(str, tmp);

    return str;
}

char* routeListToJSON(const List *list){

    if(list == NULL){
        char* str = malloc(sizeof(char) * 3);
        strcpy(str, "[]");
        return str;
    }

    int mem = 256;
    ListIterator iter = createIterator((List*)list);
    char* strTmp = malloc(sizeof(char) * mem);
    strcpy(strTmp,"[");
    int i = 0;
    for(Route* rt = nextElement(&iter); rt != NULL; rt = nextElement(&iter)){
        char* tmp = routeToJSON(rt);
        mem += 256;
        strTmp = realloc(strTmp, sizeof(char) * mem);
        strcat(strTmp, tmp);

        if(i < getLength((List*)list)-1){
            strcat(strTmp, ",");
        }
        i++;

        free(tmp);
    }
    strcat(strTmp,"]");


    return strTmp;
}


char* trackListToJSON(const List *list){


    if(list == NULL){
        char* str = malloc(sizeof(char) * 3);
        strcpy(str, "[]");
        return str;
    }

    ListIterator iter = createIterator((List*)list);
    int mem = 256;
    char* strTmp = malloc(sizeof(char) * mem);
    strcpy(strTmp,"[");
    int i = 0;
    for(Track* tr = nextElement(&iter); tr != NULL; tr = nextElement(&iter)){

        char* tmp = trackToJSON(tr);
        // printf("%s\n", tmp);
        mem += 256;
        strTmp = realloc(strTmp, sizeof(char) * mem);
        strcat(strTmp, tmp);

        if(i < getLength((List*)list)-1){
            strcat(strTmp, ",");
        }
        i++;
        free(tmp);
    }
    strcat(strTmp,"]");

    return strTmp;
}

char* GPXtoJSON(const GPXdoc* gpx){

    if(gpx == NULL){
        char* str = malloc(sizeof(char) * 3);
        strcpy(str, "{}");
        return str;
    }

    char tmp[1024];
    sprintf(tmp, "{\"version\":%.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", gpx->version, gpx->creator, getNumWaypoints(gpx), getNumRoutes(gpx), getNumTracks(gpx));

    char* str = malloc(sizeof(char) * strlen(tmp) + 1);
    strcpy(str, tmp);

    return str;
}

//bonus

void addWaypoint(Route *rt, Waypoint *pt){
    if(rt == NULL || pt == NULL){
        return;
    }

    insertBack(rt->waypoints, pt);
}

void addRoute(GPXdoc* doc, Route* rt){
    if(doc == NULL || rt == NULL){
        return;
    }

    insertBack(doc->routes, rt);
}


GPXdoc* JSONtoGPX(const char* gpxString){
    if(gpxString == NULL){
        return NULL;
    }

    GPXdoc* doc = malloc(sizeof(GPXdoc));
    doc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    doc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    doc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

    strcpy(doc->namespace, "http://www.topografix.com/GPX/1/1");
    char verTmp[100];
    char creatorTmp[256];

    char* tmp = malloc(strlen(gpxString)+1);
    strcpy(tmp, gpxString);

    char* tok = strtok(tmp, "\"");
    int i = 0;
    while(tok != NULL){
        if(i == 2){
            strcpy(verTmp, tok);
        }else if( i == 5){
            strcpy(creatorTmp, tok);
        }
        tok = strtok(NULL, "\"");
        i++;
    }

    char onlyAttr[256];
    int j = 0;
    for(int i = 1; i< strlen(verTmp) - 1; i++){
        onlyAttr[j] = verTmp[i];
        j++;
    }
    onlyAttr[j] = '\0';

    doc->version = atof(onlyAttr);

    doc->creator= malloc(strlen(creatorTmp)+1);
    strcpy(doc->creator, creatorTmp);

    free(tmp);
    return doc;
}


Waypoint* JSONtoWaypoint(const char* gpxString){

    if(gpxString == NULL){
        return NULL;
    }


    Waypoint* wpt = malloc(sizeof(Waypoint));
    wpt->name = malloc(sizeof(char)+3);
    strcpy(wpt->name,"");

    char* tmp = malloc(strlen(gpxString)+1);
    strcpy(tmp, gpxString);

    char* tok = strtok(tmp, "\"");
    int i = 0;
    char latTmp[256];
    char lonTmp[256];

    while(tok != NULL){
        if(i == 2){
            strcpy(latTmp, tok);
        }else if( i == 4){
            strcpy(lonTmp, tok);
        }
        tok = strtok(NULL, "\"");
        i++;
    }

    char onlyAttr[256];
    int j = 0;
    for(int i = 1; i< strlen(latTmp) - 1; i++){
        onlyAttr[j] = latTmp[i];
        j++;
    }
    onlyAttr[j] = '\0';
    wpt->latitude = atof(onlyAttr);

    j = 0;
    for(int i = 1; i< strlen(lonTmp) - 1; i++){
        onlyAttr[j] = lonTmp[i];
        j++;
    }
    onlyAttr[j] = '\0';

    wpt->longitude = atof(onlyAttr);

    wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    free(tmp);

    return wpt;
}



Route* JSONtoRoute(const char* gpxString){
    if(gpxString == NULL){
        return NULL;
    }


    Route* rt = malloc(sizeof(Route));

    rt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    rt->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);


    char* tmp = malloc(strlen(gpxString)+1);
    strcpy(tmp, gpxString);

    char* tok = strtok(tmp, "\"");
    int i = 0;
    char nameTmp[100];

    while(tok != NULL){
        if(i == 3){
            strcpy(nameTmp, tok);
        }
        tok = strtok(NULL, "\"");
        i++;
    }

    if(strcmp(nameTmp, "}") !=0){
        nameTmp[strlen(nameTmp)] = '\0';
        rt->name = malloc(sizeof(char) * strlen(nameTmp)+1);
        strcpy(rt->name, nameTmp);
    }else{
        rt->name = malloc(sizeof(char) * strlen(nameTmp)+1);
        strcpy(rt->name, "");
    }

    free(tmp);
    return rt;
}
