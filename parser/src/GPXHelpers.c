#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "LinkedListAPI.h"
#include "GPXHelper.h"
#include "GPXParser.h"

/*section of parseTree() also taken from libXmlExamples.c provided by the instructor*/


void setWptAttr(xmlNode *curNode, Waypoint *wptNode){
    wptNode->latitude = 0.0;
    wptNode->longitude = 0.0;
    
    xmlAttr *attr;
    for (attr = curNode->properties; attr != NULL; attr = attr->next){
        xmlNode *value = attr->children;
        char *attrName = (char *)attr->name;
        char *cont = (char *)(value->content);

        if(strcmp(attrName, "lat") == 0){
            wptNode->latitude = atof(cont);

        }else{
            wptNode->longitude = atof(cont);

        }
    }

}
 
//add wpt nodes to wpt list
void setWptChild( xmlNode* curNode, Waypoint* wptNode, List* otherData){
   
    for(xmlNode * n = curNode; n!=NULL; n = n->next){
        char* type = (char*)n->name;

        if(strcmp(type, "name") == 0 ){

            //go to name elements child, text
            char* cont = (char*)xmlNodeGetContent(n);

            if(cont != NULL){
                int len = strlen(cont);
                free(wptNode->name);
                wptNode->name = calloc(len +1, sizeof(char) );

                strcpy(wptNode->name, cont);
            }

            xmlFree(cont);

        }else if(strcmp(type, "text") != 0 ){
            
            setGPXData(n, otherData);

        }
        setWptChild(n->children, wptNode, otherData);
    }
    
}

void createWpt(xmlNode *node, List* wptList){
    Waypoint* wpt = malloc(sizeof(Waypoint));

    wpt->name = calloc(256, sizeof(char));
    strcpy(wpt->name, "");

    //init and set to List other
    wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
   
    setWptChild(node->children, wpt, wpt->otherData);
  
    setWptAttr(node, wpt);

    //add to List Waypoint 
    insertBack(wptList, wpt);
}


/************************************************************/
void createTrk(xmlNode* node, List *trkList){
    Track* trk = malloc(sizeof(Track));
    trk->name = calloc(256, sizeof(char));
    strcpy(trk->name, "");

    trk->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    trk->segments =  initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
    
    getTrackName(node->children, trk);
    setTrkChild(node->children, trk, trk->otherData, trk->segments);

    insertBack(trkList, trk); //add each track to doc->tracks   
}

void getTrackName(xmlNode *curNode, Track *rteNode){
    
    for(xmlNode * n = curNode; n!=NULL; n = n->next){
        char* type = (char*)n->name;

        if(strcmp(type, "name") == 0 ){

            //go to name elements child, text
            char* cont = (char*)xmlNodeGetContent(n);

            if(cont != NULL){
                int len = strlen(cont);
                free(rteNode->name);
                rteNode->name = calloc(len +1, sizeof(char) );
                strcpy(rteNode->name, cont);
            }

            xmlFree(cont);
        }
    }
}

void setTrkChild(xmlNode *curNode, Track *trkNode, List* otherData, List* segList){
   
    for(xmlNode * n = curNode; n!=NULL; n = n->next){
        char* type = (char*)n->name;
        
        if(strcmp(type, "trkseg") == 0){
            // printf("type in trkseg %s\n", type);
            xmlNode* tmp = n;

            TrackSegment* trkseg = malloc(sizeof(TrackSegment));
            trkseg->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
            createTrkSeg(tmp->children, trkseg->waypoints);
            
            //add each trkseg to segment list
            insertBack(segList, trkseg);  
          
        }else {

            xmlNode* tmp = xmlNextElementSibling(n->next);
            if(tmp != NULL){
                if( (strcmp((char*)tmp->name, "trkseg") != 0) && (strcmp((char*)tmp->name, "trkpt") != 0) && (strcmp((char*)n->name, "name") != 0)) {
                    setGPXData(tmp, otherData);
                }

            }
        }
        
        setTrkChild(n->children, trkNode, otherData, segList);
    }
}

void createTrkSeg(xmlNode* node, List* trkWpt){
    xmlNode * n;
    for(n = node; n != NULL; n= n->next){
        if((strcmp((char*)n->name, "trkpt") == 0)){
            
            //get waypoints and add each wpt to wptlist 
            createWpt(n, trkWpt); 
        }
    }

}

/************************************************************/
void createRte(xmlNode* node, List* rteList){
    Route* rte = malloc(sizeof(Route));
    rte->name = calloc(256, sizeof(char));
    strcpy(rte->name, "");

    rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

    getRouteName(node->children, rte);
    setRteChild(node->children, rte, rte->waypoints, rte->otherData);

    insertBack(rteList, rte);
}

void getRouteName(xmlNode *curNode, Route *rteNode){
    
    for(xmlNode * n = curNode; n!=NULL; n = n->next){
        char* type = (char*)n->name;

        if(strcmp(type, "name") == 0 ){

            //go to name elements child, text
            char* cont = (char*)xmlNodeGetContent(n);

            if(cont != NULL){
                int len = strlen(cont);
                free(rteNode->name);
                rteNode->name = calloc(len +1, sizeof(char) );
                strcpy(rteNode->name, cont);
            }

            xmlFree(cont);
        }
    }
}


void setRteChild(xmlNode *curNode, Route *rteNode, List* wptList, List* otherData){

    for(xmlNode * n = curNode; n!=NULL; n = n->next){
        // char* type = (char*)n->name;

        if((strcmp((char*)n->name, "rtept") == 0)){
            createWpt(n, wptList);

        }else if((strcmp((char*)n->name, "text") != 0) && (strcmp((char*)n->parent->name, "rtept") != 0) && (strcmp((char*)n->name, "name") != 0)){
            setGPXData(n, otherData);
            
        }
       
        setRteChild(n->children, rteNode, wptList, otherData);

    }
}


void parseTree(xmlNode *node, GPXdoc *doc){

    for(xmlNode * n = node; n!=NULL; n = n->next){
        char* type = (char*)n->name;

        if(strcmp(type, "wpt" ) == 0){
            createWpt(n, doc->waypoints);

        }else if(strcmp(type, "trk" ) == 0){
    
            createTrk(n, doc->tracks);
        }else if(strcmp(type, "rte" ) == 0){
    
            createRte(n, doc->routes);
        }
    
        parseTree(n->children, doc);

    }
  
}
 

// gpxdata, check name of node, names that arent main ones get those
void setGPXData(xmlNode* node, List* otherData){

    int contLength;
    GPXData* data;

    if(node != NULL){
        char* name = (char*)node->name;

        char* cont = (char*)xmlNodeGetContent(node);
        if(cont != NULL){
            contLength = strlen(cont);
        }

        //init GPXData
        data = malloc(sizeof(GPXData) + sizeof(char*)*(contLength +1));
        
        if(name == NULL){
            strcpy(data->name, "name is NULL");

        }else if(strlen(name) == 0){
            strcpy(data->name, "name is empty");

        }else{
            //set name
            strcpy(data->name, name);
        }

        if(cont == NULL){
            strcpy(data->value, "value is NULL");

        }else if(strlen(cont) == 0){
            strcpy(data->value, "value is empty");

        }else{
            //set value
            strcpy(data->value, cont);

        }
       
        xmlFree(cont);

        insertBack(otherData, data);
    }

}


