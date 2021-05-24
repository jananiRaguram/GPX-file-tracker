#ifndef GPX_HELPER_H
#define GPX_HELPER_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "LinkedListAPI.h"
#include "GPXParser.h"


void parseTree(xmlNode *node, GPXdoc *doc);
void setGPXData(xmlNode* node, List*otherData);

void createTrk(xmlNode* node, List *trkList );
void setTrkChild(xmlNode *curNode, Track *trkNode, List* otherData, List* segList);

void createWpt(xmlNode *node, List *wptList);
void setWptChild( xmlNode* curNode, Waypoint* wptNode, List* otherData);
void setWptAttr(xmlNode *curNode, Waypoint *wptNode);

void setRteChild(xmlNode *curNode, Route *rteNode, List* wptList, List* otherData);
void createRte(xmlNode* node, List *rteList);

void createTrkSeg(xmlNode* node, List* trkWpt);

bool findTrack(const void* a, const void* b);
bool findRoute(const void* a, const void* b);
bool findWaypoint(const void* a, const void* b);

void getRouteName(xmlNode *curNode, Route *rteNode);
void getTrackName(xmlNode *curNode, Track* trkNode);


bool validateFile(xmlDoc* doc, char* gpxSchemaFile);
xmlNode* convertGpxNodes(GPXdoc* doc, xmlDoc* xmlDoc );
void deleteList(void* data);
double pointDistance(double srcLat, double srcLon, double destLat, double destLon);

char* getGPX_JSON(char* fileName);
char* getJSON_trList(char* fileName);

//gpx view panel otherData getting
char* getJSON_otherList(char* fileName, char* type, char* str);
char* trOtherListToJSON(char* str, List* list);
char* otherToJSON(GPXData* other);
char* rtOtherListToJSON(char* tStr, List* list);

char* validateFileCheck(char* fileName, char* gpxSchemaFile);
int renameCmp(char* fileName, char* newName, int typeNum, char* type);

char* rtrim(char *s);

char* rtsBtwnToJSON( List* rtList);
char* trksBtwnToJSON( List* trList);
char* trkPathBtwnToJSON(char* fileName, float startLat, float startLon, float endLat, float endLon, float delta );
char* rtPathBtwnToJSON(char* fileName, float startLat, float startLon, float endLat, float endLon, float delta );

int createFile(char* fileName, char* both);
int addRt(char* fileName, char* wptStr, char* rtStr );

char* rtPtsToJSON(char* fileName);

char* wptToJSON(const Waypoint* wpt, int pntNum);


#endif
