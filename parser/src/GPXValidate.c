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

//validating with schema file reference used: http://knol2share.blogspot.com/2009/05/validate-xml-against-xsdin-c.html
//validate libxml tree
bool validateFile(xmlDoc* doc, char* gpxSchemaFile){

    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;

    ctxt = xmlSchemaNewParserCtxt(gpxSchemaFile);

    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);

    xmlSchemaValidCtxtPtr ctxt2;
    int ret;

    ctxt2 = xmlSchemaNewValidCtxt(schema);
    ret = xmlSchemaValidateDoc(ctxt2, doc);

    xmlSchemaFreeValidCtxt(ctxt2);

    if(schema != NULL){
        xmlSchemaFree(schema);
    }

    xmlSchemaCleanupTypes();

    if(ret != 0){
        return false;
    }

    return true;
}


/*createGPXdoc uses sample code from "main()" and "print_element_names()"
from https://www.xmlsoft.org/examples/tree1.c to parse the XML file */

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile){


    /*parsing XML file*/
    xmlDoc *doc; //XML tree stored
    xmlNode *rootElement;
    xmlNode *curNode;

    doc = xmlReadFile(fileName, NULL, 0);
    if(doc == NULL){
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }else{
         //function to validate xml agaist schema
         //private functino that does that, takes in a libxmltree and schema file name
        bool ret = validateFile(doc, gpxSchemaFile);

        if(ret == false){
            xmlFreeDoc(doc);
            xmlCleanupParser();
            return NULL;
        }
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
    strcpy(structPtr->creator, "No creator");

    structPtr->version = 1.1;

    //get namespace
    if(rootElement->ns == NULL){

        // strcpy(structPtr->namespace, "namespace is NULL");
        deleteGPXdoc(structPtr);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }else if(strlen((char*)rootElement->ns->href) == 0){
        // strcpy(structPtr->namespace, "namespace is empty");
        deleteGPXdoc(structPtr);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
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

    //creator can't be empty string
    if(strlen(structPtr->creator) == 0){
        deleteGPXdoc(structPtr);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    //go through elements wpt, trk, rte and initialze them
    parseTree(rootElement, structPtr);

    xmlFreeDoc(doc);
    xmlCleanupParser();
    return structPtr;
}


bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile){

    if(doc == NULL || gpxSchemaFile == NULL){
      return false;
    }
    //check for invalid ones and return false
    char* fileType = strrchr(gpxSchemaFile, '.');
    if(strcmp(fileType, ".xsd") != 0 ){
      return false;
    }


    //manually check against GPXparser.h
    if(doc->tracks == NULL || doc->routes == NULL || doc->waypoints == NULL){
        return false;
    }

    if(doc->creator == NULL || strlen(doc->namespace) == 0){
        return false;
    }

    if(strlen(doc->creator) == 0){
        return false;
    }

    //check if waypoint list correctly initialized 
    ListIterator iterWpt = createIterator(doc->waypoints);

    for(Waypoint* getWpt = nextElement(&iterWpt); getWpt != NULL; getWpt= nextElement(&iterWpt)){
       if(getWpt->name == NULL){
           return false;
       }else if(getWpt->otherData == NULL){
           return false;
       }
       
        ListIterator iterOther = createIterator(getWpt->otherData);
        for(GPXData* data = nextElement(&iterOther); data != NULL; data = nextElement(&iterOther)){
            if(strlen(data->name) == 0){
                return false;
            }else if (strlen(data->value) == 0){
                return false;
            }
        }

    }

    //check route list correctly initialized
    ListIterator iterRt = createIterator(doc->routes);

    for(Route* rt = nextElement(&iterRt); rt != NULL; rt = nextElement(&iterRt)){
        if(rt->waypoints == NULL){
            return false;
        }else if(rt->name == NULL){
            return false;
        }else if(rt->otherData == NULL){
            return false;
        }

        iterWpt = createIterator(rt->waypoints);

        for(Waypoint* getWpt = nextElement(&iterWpt); getWpt != NULL; getWpt= nextElement(&iterWpt)){
            if(getWpt->name == NULL){
                return false;
            }else if(getWpt->otherData == NULL){
                return false;
            }
        
            ListIterator iterOther = createIterator(getWpt->otherData);
            for(GPXData* data = nextElement(&iterOther); data != NULL; data = nextElement(&iterOther)){
                if(strlen(data->name) == 0){
                    return false;
                }else if (strlen(data->value) == 0){
                    return false;
                }
            }

        }
    }

    //check route list correctly initialized
    ListIterator iterTr = createIterator(doc->tracks);

    for(Track* tr = nextElement(&iterTr); tr != NULL; tr = nextElement(&iterTr)){
        if(tr->name == NULL){
            return false;
        }else if(tr->segments == NULL){
            return false;
        }else if(tr->otherData == NULL){
            return false;
        }

        ListIterator iterTrSeg = createIterator(tr->segments);
        for(TrackSegment* seg = nextElement(&iterTrSeg); seg!=NULL; seg = nextElement(&iterTrSeg)){
            if(seg->waypoints == NULL){
                return false;
            }

            iterWpt = createIterator(seg->waypoints);

            for(Waypoint* getWpt = nextElement(&iterWpt); getWpt != NULL; getWpt= nextElement(&iterWpt)){
                if(getWpt->name == NULL){
                    return false;
                }else if(getWpt->otherData == NULL){
                    return false;
                }
            
                ListIterator iterOther = createIterator(getWpt->otherData);
                for(GPXData* data = nextElement(&iterOther); data != NULL; data = nextElement(&iterOther)){
                    if(strlen(data->name) == 0){
                        return false;
                    }else if (strlen(data->value) == 0){
                        return false;
                    }
                }

            }
        }

    }

    

    //get tree
    xmlDoc* xmlDoc = NULL;
    xmlNode* root = NULL;

    xmlDoc = xmlNewDoc(BAD_CAST "1.0");
    root = convertGpxNodes(doc, xmlDoc);
    xmlDocSetRootElement(xmlDoc, root);

    bool ret = validateFile(xmlDoc, gpxSchemaFile);
    if(ret == false){
        xmlFreeDoc(xmlDoc);
        xmlCleanupParser();
        return false;
    }

    xmlFreeDoc(xmlDoc);
    xmlCleanupParser();

    return true;
}


bool writeGPXdoc(GPXdoc* doc, char* fileName){

    //check for invalid ones and return false
    if(doc == NULL || fileName == NULL){
        return false;
    }

    char* fileType = strrchr(fileName, '.');
    if(strcmp(fileType, ".xml") != 0 && strcmp(fileType, ".gpx") != 0 ){
      return false;
    }


    xmlDoc* xmlDoc = NULL;
    xmlNode* root = NULL;

    xmlDoc = xmlNewDoc(BAD_CAST "1.0");
    root = convertGpxNodes(doc, xmlDoc);
    xmlDocSetRootElement(xmlDoc, root);

    int ret = xmlSaveFormatFileEnc(fileName, xmlDoc, "UTF-8", 1);
    if(ret == -1){
        xmlFreeDoc(xmlDoc);
        xmlCleanupParser();
        return false;
    }
    xmlFreeDoc(xmlDoc);
    xmlCleanupParser();
    return true;
}


xmlNode* convertGpxNodes(GPXdoc* doc, xmlDoc* xmlDoc ){
    xmlNode* root = xmlNewNode(NULL, BAD_CAST "gpx");
    xmlDocSetRootElement(xmlDoc, root);

    xmlNewProp(root, BAD_CAST "creator", BAD_CAST doc->creator);

    char vers[100];
    sprintf(vers, "%.1f", doc->version);
    xmlNewProp(root, BAD_CAST "version", BAD_CAST vers);


    xmlNsPtr ns = xmlNewNs(root, BAD_CAST doc->namespace, NULL);
    xmlSetNs(root, ns);

    xmlNode* node = xmlNewChild(root, NULL, NULL, NULL);



    ListIterator iter = createIterator(doc->waypoints);
    char type[100] = "wpt";
    char lat[256];
    char lon[256];


    //write waypoints
    for(Waypoint* getWpt = nextElement(&iter); getWpt != NULL; getWpt= nextElement(&iter)){
        node = xmlNewChild(root, NULL, BAD_CAST type, NULL);
        sprintf(lat, "%f", getWpt->latitude);
        sprintf(lon, "%f", getWpt->longitude);

        xmlNewProp(node, BAD_CAST "lat" ,BAD_CAST lat);
        xmlNewProp(node, BAD_CAST "lon" ,BAD_CAST lon);

        xmlNodePtr node1, txt = NULL;

        //get name if not empty string
        if(strlen(getWpt->name) != 0){
            node1 = xmlNewChild(node, NULL, BAD_CAST "name", NULL);
            txt = xmlNewText(BAD_CAST getWpt->name);
            xmlAddChild(node1, txt);
        }

        //get other data
        ListIterator otherIter = createIterator(getWpt->otherData);
        for(GPXData* otherData = nextElement(&otherIter); otherData !=NULL; otherData = nextElement(&otherIter) ){
            node1 = xmlNewChild(node, NULL, BAD_CAST otherData->name, NULL);
            txt = xmlNewText(BAD_CAST otherData->value);
            xmlAddChild(node1, txt);
        }

    }



    //write routes
    iter = createIterator(doc->routes);
    strcpy(type, "rte");
    for(Route* getRte = nextElement(&iter); getRte != NULL; getRte= nextElement(&iter)){
        node = xmlNewChild(root, NULL, BAD_CAST type, NULL);
        xmlNodePtr node1, txt = NULL;

        //get name if not empty string
        if(strlen(getRte->name) != 0){
            node1 = xmlNewChild(node, NULL, BAD_CAST "name", NULL);
            txt = xmlNewText(BAD_CAST getRte->name);
            xmlAddChild(node1, txt);
        }

        //route other data
        ListIterator otherIter = createIterator(getRte->otherData);
        for(GPXData* otherData = nextElement(&otherIter); otherData !=NULL; otherData = nextElement(&otherIter) ){
            node1 = xmlNewChild(node, NULL, BAD_CAST otherData->name, NULL);
            txt = xmlNewText(BAD_CAST otherData->value);
            xmlAddChild(node1, txt);
        }

        //get route waypoints
        ListIterator iterWpt = createIterator(getRte->waypoints);
        for(Waypoint* getWpt = nextElement(&iterWpt); getWpt != NULL; getWpt= nextElement(&iterWpt)){
            xmlNodePtr node2, txt2 = NULL;

            node1 = xmlNewChild(node, NULL, BAD_CAST "rtept", NULL);
            sprintf(lat, "%f", getWpt->latitude);
            sprintf(lon, "%f", getWpt->longitude);

            xmlNewProp(node1, BAD_CAST "lat" ,BAD_CAST lat);
            xmlNewProp(node1, BAD_CAST "lon" ,BAD_CAST lon);

            if(strlen(getWpt->name) != 0){
                node2 = xmlNewChild(node1, NULL, BAD_CAST "name", NULL);
                txt2 = xmlNewText(BAD_CAST getWpt->name);
                xmlAddChild(node2, txt2);
            }

            //waypoints otherdata
            otherIter = createIterator(getWpt->otherData);
            for(GPXData* otherData = nextElement(&otherIter); otherData !=NULL; otherData = nextElement(&otherIter) ){
                node2 = xmlNewChild(node1, NULL, BAD_CAST otherData->name, NULL);
                txt2 = xmlNewText(BAD_CAST otherData->value);
                xmlAddChild(node2, txt2);
            }

        }
    }


    //write tracks
    iter = createIterator(doc->tracks);
    strcpy(type, "trk");
    for(Track* getTrk = nextElement(&iter); getTrk != NULL; getTrk = nextElement(&iter)){
        node = xmlNewChild(root, NULL, BAD_CAST type, NULL);
        xmlNodePtr node1, node2, node3, node4, txt, txt2 = NULL;

        //get name if not empty string
        if(strlen(getTrk ->name) != 0){
            node1 = xmlNewChild(node, NULL, BAD_CAST "name", NULL);
            txt = xmlNewText(BAD_CAST getTrk->name);
            xmlAddChild(node1, txt);
        }

        //get track other data
        ListIterator otherIter = createIterator(getTrk ->otherData);
        for(GPXData* otherData = nextElement(&otherIter); otherData !=NULL; otherData = nextElement(&otherIter) ){
            node1 = xmlNewChild(node, NULL, BAD_CAST otherData->name, NULL);
            txt = xmlNewText(BAD_CAST otherData->value);
            xmlAddChild(node1, txt);
        }


        //get track segments
        ListIterator iterTrkSeg = createIterator(getTrk->segments);
        for(TrackSegment* getTrkSeg = nextElement(&iterTrkSeg ); getTrkSeg != NULL; getTrkSeg = nextElement(&iterTrkSeg )){
            node2 = xmlNewChild(node, NULL, BAD_CAST "trkseg", NULL);

            //waypoints in each segment
            ListIterator iterWpt = createIterator(getTrkSeg->waypoints);
            for(Waypoint* getWpt = nextElement(&iterWpt); getWpt != NULL; getWpt= nextElement(&iterWpt)){

                node3 = xmlNewChild(node2, NULL, BAD_CAST "trkpt", NULL);
                sprintf(lat, "%f", getWpt->latitude);
                sprintf(lon, "%f", getWpt->longitude);

                xmlNewProp(node3, BAD_CAST "lat" ,BAD_CAST lat);
                xmlNewProp(node3, BAD_CAST "lon" ,BAD_CAST lon);

                if(strlen(getWpt->name) != 0){
                    node4 = xmlNewChild(node3, NULL, BAD_CAST "name", NULL);
                    txt2 = xmlNewText(BAD_CAST getWpt->name);
                    xmlAddChild(node4, txt2);
                }

                //waypoint otherdata
                otherIter = createIterator(getWpt->otherData);
                for(GPXData* otherData = nextElement(&otherIter); otherData !=NULL; otherData = nextElement(&otherIter) ){
                    node4 = xmlNewChild(node3, NULL, BAD_CAST otherData->name, NULL);
                    txt2 = xmlNewText(BAD_CAST otherData->value);
                    xmlAddChild(node4, txt2);
                }

            }

        }
    }

    return root;
}
