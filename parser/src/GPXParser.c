#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "GPXParser.h"
#include "GPXHelper.h"
#include "LinkedListAPI.h"

/*createGPXdoc uses sample code from "main()" and "print_element_names()"
from https://www.xmlsoft.org/examples/tree1.c to parse the XML file */


GPXdoc* createGPXdoc(char* filename){
    /*parsing XML file*/
    xmlDoc *doc; //XML tree stored
    xmlNode *rootElement;
    xmlNode *curNode;

    doc = xmlReadFile(filename, NULL, 0);
    if(doc == NULL){
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    //root node of XML file
    rootElement = xmlDocGetRootElement(doc);


    //allocate space for GPX struct to store all elements
    GPXdoc *structPtr = malloc(sizeof(GPXdoc));
    if(structPtr == NULL){
        free(structPtr);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    structPtr->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    structPtr->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    structPtr->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

    structPtr->creator = calloc(256, sizeof(char));
    // strcpy(structPtr->creator, "No creator");

    // structPtr->version = 1.1;

    //get namespace
    if(rootElement->ns == NULL){

        // strcpy(structPtr->namespace, "namespace is NULL");

    }else if(strlen((char*)rootElement->ns->href) == 0){
        // strcpy(structPtr->namespace, "namespace is empty");

    }else{
        char* ns = (char*)rootElement->ns->href;
        strcpy(structPtr->namespace, ns);
    }


    for (curNode = rootElement; curNode != NULL; curNode = curNode->next) {

        curNode = rootElement;
        xmlAttr *attr;

        for (attr = curNode->properties; attr != NULL; attr = attr->next){

            xmlNode *value = attr->children;
            char *attrName = (char *)attr->name;
            char *cont = (char *)(value->content);
            //assign struct values for values for corresponding gpx attributes

            if(strcmp(attrName, "version") == 0){
                structPtr->version = atof(cont);

            }else if(strcmp(attrName, "creator") == 0){

                strcpy(structPtr->creator, cont);
            }

        }



    }

    //go through elements wpt, trk, rte and initialze them
    parseTree(rootElement, structPtr);

    xmlFreeDoc(doc);
    xmlCleanupParser();
    return structPtr;

}


void deleteGPXdoc(GPXdoc* doc){

    if(doc == NULL){
        return;
    }
    free(doc->creator);
    freeList(doc->waypoints);
    freeList(doc->routes);
    freeList(doc->tracks);
    free(doc);
}


/*****************************************************/
char* GPXdocToString(GPXdoc* doc){

    if(doc == NULL){
        return NULL;
    }
    char* str = malloc(sizeof(char*) * (doc->version + strlen(doc->creator) + strlen(doc->namespace) + 100));
    sprintf(str, "In GPX doc:\n\nVersion: %.2f\nCreator: %s\nNamespace:%s\n", doc->version, doc->creator, doc->namespace );


    if(getLength(doc->waypoints) != 0){

        strcat(str, "\nWaypoints:");
        char* wptStr = toString(doc->waypoints);
        str = realloc(str, sizeof(char*) * strlen(wptStr));

        strcat(str, wptStr);
        free(wptStr);
    }

    if(getLength(doc->tracks) != 0){

        strcat(str, "\nTracks:\n");
        char* trkStr = toString(doc->tracks);
        str = realloc(str, sizeof(char*) * strlen(trkStr));

        strcat(str, trkStr);
        free(trkStr);
    }

    if(getLength(doc->routes) != 0){

        strcat(str, "\nRoutes:\n");
        char* rteStr = toString(doc->routes);
        str = realloc(str, sizeof(char*) * (strlen(rteStr)));
        strcat(str,rteStr);
        free(rteStr);
    }

    return str;
}

/****************************************/
//A3 functions

//print to file panel log
char* getGPX_JSON(char* fileName){
    GPXdoc* doc = createGPXdoc(fileName);
    char* str;
    str = GPXtoJSON(doc);

    if(doc != NULL){
        deleteGPXdoc(doc);
    }

    return str;
}

//get list of routes in JSON
char* getJSON_rtList(char* fileName){
    GPXdoc* doc = createGPXdoc(fileName);
    char* str;
    str = routeListToJSON(doc->routes);

    if(doc != NULL){
        deleteGPXdoc(doc);
    }
    return str;
}

//check validity of a file
char* validateFileCheck(char* fileName, char* gpxSchemaFile){
    GPXdoc* doc = createValidGPXdoc(fileName, gpxSchemaFile);

    if(doc == NULL){
        return "false";
    }else{
        return "true";
    }
}

//get list of routes in JSON
char* getJSON_trList(char* fileName){
    GPXdoc* doc = createGPXdoc(fileName);
    char* str;
    str = trackListToJSON(doc->tracks);
    if(doc != NULL){
        deleteGPXdoc(doc);
    }
    return str;
}

char* getJSON_otherList(char* fileName, char* type, char* tStr){
    GPXdoc* doc = createGPXdoc(fileName);
    char* str;
    if(strcmp(type, "track") == 0){
        str = trOtherListToJSON(tStr, doc->tracks);
    }else{
        str = rtOtherListToJSON(tStr, doc->routes);
    }
    if(doc != NULL){
        deleteGPXdoc(doc);
    }

    return str;
}

char* trOtherListToJSON(char* tStr, List* list){

    if(list == NULL){
        char* str = malloc(sizeof(char) * 3);
        strcpy(str, "[]");
        return str;
    }else if(getLength((List*)list) == 0){
        char* str = malloc(sizeof(char) * 3);
        strcpy(str, "[]");
        return str;
    }

    int mem = 256;
    ListIterator iter = createIterator((List*)list);
    char* strTmp = malloc(sizeof(char) *mem);
    strcpy(strTmp,"[");

    for(Track* tr = nextElement(&iter); tr != NULL; tr = nextElement(&iter)){
        char* trTmp = trackToJSON(tr);

        if(strcmp(trTmp, tStr) == 0){
            ListIterator iterOther = createIterator(tr->otherData);
            for(GPXData* other = nextElement(&iterOther); other != NULL; other = nextElement(&iterOther)){
                char* tmp = otherToJSON(other);
                mem += 256;
                strTmp = realloc(strTmp, sizeof(char) * mem);
                strcat(strTmp, tmp);

                ListIterator checkNext = iterOther;
                if(nextElement(&checkNext) != NULL){
                    strncat(strTmp, ",",2);
                }
                iterOther = checkNext;
                free(tmp);
            }
        }

        free(trTmp);
    }
    strncat(strTmp, "]",2);

    return strTmp;
}

char* rtOtherListToJSON(char* tStr, List* list){

    if(list == NULL){
        char* str = malloc(sizeof(char) * 3);
        strcpy(str, "[]");
        return str;
    }else if(getLength((List*)list) == 0){
        char* str = malloc(sizeof(char) * 3);
        strcpy(str, "[]");
        return str;
    }

    int mem = 256;
    ListIterator iter = createIterator((List*)list);
    char* strTmp = malloc(sizeof(char) *mem);
    strcpy(strTmp,"[");

    for(Route* rt = nextElement(&iter); rt != NULL; rt = nextElement(&iter)){
        char* rtTmp = routeToJSON(rt);

        if(strcmp(rtTmp, tStr) == 0){
            ListIterator iterOther = createIterator(rt->otherData);
            for(GPXData* other = nextElement(&iterOther); other != NULL; other = nextElement(&iterOther)){
                char* tmp = otherToJSON(other);
                mem += 256;
                strTmp = realloc(strTmp, sizeof(char) * mem);
                strcat(strTmp, tmp);

                ListIterator checkNext = iterOther;
                if(nextElement(&checkNext) != NULL){
                    strncat(strTmp, ",",2);
                }
                iterOther = checkNext;
                free(tmp);
            }
        }

        free(rtTmp);
    }
    strncat(strTmp, "]",2);

    return strTmp;
}

char* otherToJSON(GPXData* other){
    char* str;

    if(other == NULL){
        str = malloc(sizeof(char) *3);
        strcpy(str, "{}");
        return str;
    }

    str = malloc(sizeof(char) * strlen(other->name) + strlen(other->value) + 100);
    sprintf(str, "{\"name\":\"%s\",\"value\":\"%s\"}", other->name, rtrim(other->value) );

    return str;
}

char* rtrim(char *s){

    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';

    return s;
}


char* trkPathBtwnToJSON(char* fileName, float startLat, float startLon, float endLat, float endLon, float delta ){
    GPXdoc* doc = createGPXdoc(fileName);
    // printf("stuff %f %f %f %f %f\n", startLat, startLon, endLat, endLon, delta);

    List* trList = getTracksBetween(doc, startLat, startLon, endLat, endLon, delta);
    char* trksJSON = trksBtwnToJSON(trList);

    return trksJSON;
}

char* rtPathBtwnToJSON(char* fileName, float startLat, float startLon, float endLat, float endLon, float delta ){
    GPXdoc* doc = createGPXdoc(fileName);
    // if(doc == NULL){
    //     printf("NULL");
    // }
    // printf("stuff %f %f %f %f %f\n", startLat, startLon, endLat, endLon, delta);
    List* rtList = getRoutesBetween(doc, startLat, startLon, endLat, endLon, delta);

    char* rtsJSON = rtsBtwnToJSON(rtList);

    return rtsJSON;
}

char* rtsBtwnToJSON( List* rtList){

    if(rtList == NULL){
        char* str = malloc(sizeof(char) * 3);
        strcpy(str, "[]");
        return str;
    }

    char* strTmp = toString(rtList);
    char* str = malloc(sizeof(char) * strlen(strTmp) + 10);
    strcpy(str, "[");

    ListIterator iter = createIterator(rtList);
    int i = 0;
    for(Route* rt = nextElement(&iter); rt != NULL; rt = nextElement(&iter)){
        char* strTmp = routeToJSON(rt);
        strcat(str, strTmp);

        if(i < getLength((List*)rtList)-1){
            strcat(str, ",");
        }
        i++;
        free(strTmp);
    }
    strcat(str, "]");
    // printf(" in rts btwn to JSON %s\n",str);

    return str;
}

char* trksBtwnToJSON( List* trList){

    if(trList == NULL){
        char* str = malloc(sizeof(char) * 3);
        strcpy(str, "[]");
        return str;
    }


    char* strTmp = toString(trList);
    char* str = malloc(sizeof(char) * strlen(strTmp) + 10);

    strcpy(str, "[");
    ListIterator iter = createIterator(trList);
    int i = 0;
    for(Track* tr = nextElement(&iter); tr != NULL; tr = nextElement(&iter)){
        char* strTmp = trackToJSON(tr);
        strcat(str, strTmp);

        if(i < getLength((List*)trList)-1){
            strcat(str, ",");
        }
        i++;
        free(strTmp);
    }

    strcat(str, "]");
    return str;
}


int createFile(char* fileName, char* both){

    GPXdoc *doc = JSONtoGPX(both);
    bool res = writeGPXdoc(doc, fileName);
    return res;
    // printf("%d \n", res );
}

int addRt(char* fileName, char* wptStr, char* rtStr ){

    GPXdoc* doc = createGPXdoc(fileName);


    Route* rt = JSONtoRoute(rtStr);
    char** str = malloc(strlen(wptStr) +1);

    char* tok = strtok(wptStr, "|");
    int i =0;
    while(tok != NULL){
        str[i] = malloc(strlen(tok) + 1);
        strcpy(str[i], tok);
        tok = strtok(NULL, "|");
        i++;
    }

    int j = 0;
    while(j < i){
        Waypoint* wpt = JSONtoWaypoint(str[j]);
        addWaypoint(rt, wpt);
        j++;
    }


    addRoute(doc, rt);
    bool res = writeGPXdoc(doc,fileName);
    if(res == false){
        free(str);
    }else{
        for(int k = 0; k < i; k++){
            free(str[k]);
        }
        free(str);
    }

    return res;
}


int renameCmp(char* fileName, char* newName, int typeNum, char* type){
    GPXdoc* doc = createGPXdoc(fileName);

    ListIterator iter;
    int i = 1;
    if(strcmp(type, "rt") == 0){

        iter = createIterator(doc->routes);
        for(Route* rt = nextElement(&iter); rt != NULL; rt = nextElement(&iter)){
            if(i == typeNum){
                rt->name = malloc(strlen(newName) + 10);
                strcpy(rt->name, newName);
                break;
            }
            i++;
        }
    }else if(strcmp(type, "trk") == 0){
        iter = createIterator(doc->tracks);
        for(Track* trk = nextElement(&iter); trk != NULL; trk = nextElement(&iter)){
            if(i == typeNum){
                trk->name = malloc(strlen(newName) + 10);
                strcpy(trk->name, newName);
                break;
            }
            i++;
        }
    }

    int res = writeGPXdoc(doc, fileName);

    if(res == 0){
        return 0;
    }else if(typeNum > i){
        return 0;
    }

    return 1;
}


char* rtPtsToJSON(char* fileName){
    GPXdoc* doc = createGPXdoc(fileName);

    ListIterator iter = createIterator(doc->routes);
    int mem= 256;
    char* str = malloc(sizeof(char) * mem);
    strcpy(str, "[");
    int j = 0;
    for(Route* rt  = nextElement(&iter); rt != NULL; rt = nextElement(&iter)){
        ListIterator wptIter = createIterator(rt->waypoints);
        int i = 0;
        for(Waypoint * wpt = nextElement(&wptIter); wpt != NULL; wpt = nextElement(&wptIter)){
            char* tmp = wptToJSON(wpt, i);
            mem += 256;
            str  = realloc(str, sizeof(char) * mem);
            strcat(str, tmp);


            if(i < getLength(rt->waypoints) -1){
                strcat(str, ",");
            }
            free(tmp);
            i++;
        }
        if(j < getLength(doc->routes) -1){
            strcat(str, ",");
        }
        j++;
    }

    strcat(str, "]");

    return str;
}


char* wptToJSON(const Waypoint* wpt, int pntNum){
    if(wpt == NULL){

        char* str = malloc(sizeof(char) * 3);
        strcpy(str, "{}");
        return str;
    }

    char name[256];
    if(strlen(wpt->name) == 0){
        strcpy(name, "None");
    }else{
        strcpy(name, wpt->name);
    }

    char lat[256];
    sprintf(lat, "%f", wpt->latitude);

    char lon[256];
    sprintf(lon, "%f", wpt->longitude);

    char num[256];
    sprintf(num, "%d", pntNum);

    int len = strlen(lat) + strlen(lon) + strlen(name) + strlen(num) + 100;
    char tmp[len];
    sprintf(tmp, "{\"pointNum\":%s,\"lat\":%s,\"lon\":%s,\"name\":\"%s\"}", num, lat, lon, name);

    char* str = malloc(sizeof(char) * strlen(tmp) + 100);
    strcpy(str, tmp);

    return str;
}
