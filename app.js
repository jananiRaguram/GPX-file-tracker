'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

const mysql = require('mysql2/promise');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
// app.post('/upload', function(req, res) {

//   if(!req.files) {
//     return res.status(400).send('No files were uploaded.');
//   }
//   let uploadFile = req.files.uploadFile;
//   fs.exists('uploads/' + uploadFile.name, (exists)=>{

//     if(exists){
//       // return res.status(400).send('files already uploaded.');
//       console.log("already exits");
//       // return;
//     }else{

//      // Use the mv() method to place the file somewhere on your server
//       uploadFile.mv('uploads/' + uploadFile.name, function(err) {
//         if(err) {
//           return res.status(500).send(err);
//         }

//         res.redirect('/');
//       });
//     }
//   })

// });

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {

  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
  let uploadFile = req.files.uploadFile;

  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });

});


//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

//******************** Your code goes here ********************
let GPXdataElements = ffi.Library('./libgpxparser.so', {
    "getGPX_JSON": ['string', ['string']],
    "getJSON_rtList": ['string', ['string']],
    "getJSON_trList": ['string', ['string']],
    "getJSON_otherList": ['string', ['string', 'string', 'string']],
    "validateFileCheck": ['string', ['string', 'string']],
    "rtPathBtwnToJSON": ['string', ['string', 'float', 'float', 'float', 'float', 'float']],
    "trkPathBtwnToJSON": ['string', ['string', 'float', 'float', 'float', 'float', 'float']],
    "createFile": ['int', ['string', 'string']],
    "addRt": ['int', ['string', 'string', 'string', 'int']],
    "renameCmp":['int', ['string', 'string', 'int', 'string']],
    "rtPtsToJSON": ['string', ['string']]

});
// let connection;


//Sample endpoint
app.get('/endpoint1', function(req , res){
  let retStr = req.query.stuff + " " + req.query.junk;
  res.send({
    stuff: retStr
  });
});

app.get('/getLogRow', function(req, res){

    let fileName = req.query.file;
    let row = GPXdataElements.getGPX_JSON(path.join(__dirname+'/uploads/')+fileName);
    let elem = JSON.parse(row);
    // console.log(fileName + " file");
    res.send({
        ver: elem.version,
        creator: elem.creator,
        numWpt: elem.numWaypoints,
        numRt: elem.numRoutes,
        numTr: elem.numTracks
    });

});

app.get('/allFiles', function(req, res){
    let files= [];

    fs.readdirSync(path.join(__dirname+'/uploads/')).forEach(file => {

      if(path.extname(file) == '.gpx'){
        files.push(file);
      }
	  })
    res.send({
        allFiles: files
    });
});

app.get('/validate', function(req, res){

    let fileName = req.query.file;
    // console.log(fileName);
    let validate = GPXdataElements.validateFileCheck(path.join(__dirname+'/uploads/')+fileName, path.join(__dirname+'/gpx.xsd') );
    res.send({
        isValid: validate
    });
});

app.get('/getRt', function(req, res){
    let fileName =  req.query.file;
    // console.log( "getRt for file" + fileName);

    let rt = GPXdataElements.getJSON_rtList(path.join(__dirname+'/uploads/')+fileName);
    // console.log("rt " + rt);

    let parsed = JSON.parse(rt);

    res.send(parsed);

});

app.get('/getTr', function(req, res){
    let fileName =  req.query.file;
    // console.log(fileName + " file");

    let tr = GPXdataElements.getJSON_trList(path.join(__dirname+'/uploads/')+fileName);
    // console.log("tr " + tr);

    let parsed = JSON.parse(tr);

    res.send(parsed);

});

app.get('/getOther', function(req, res){
    let fileName =  req.query.file;
    let kind =  req.query.type;
    let str = req.query.str;

    // console.log(fileName + " file");
    // console.log("find other for\n" + str);
    // console.log("in other");

    let tr = GPXdataElements.getJSON_otherList(path.join(__dirname+'/uploads/')+fileName, kind, str);
    // console.log("other " + tr);
    let parsed = JSON.parse(tr);
    res.send(parsed);

});

function validateFile(fileName){
    let isValid = GPXdataElements.validateFileCheck(path.join(__dirname+'/uploads/')+fileName);
    return isValid;
}

app.get('/getRtBtwnPath', function(req, res){
    let file = req.query.file;
    let sLat = req.query.sLat;
    let sLon = req.query.sLon;

    let eLat = req.query.eLat;
    let eLon = req.query.eLon;

    let delta = req.query.dlt;
    // console.log(sLat + " " + sLon + " " + eLat + " " + eLon);
    let elm = GPXdataElements.rtPathBtwnToJSON(path.join(__dirname+'/uploads/')+file, sLat, sLon, eLat, eLon, delta);
    let parsed = JSON.parse(elm);

    // console.log("elm" + elm);
    res.send(elm);

});

app.get('/getTrkBtwnPath', function(req, res){
    let file = req.query.file;
    let sLat = req.query.sLat;
    let sLon = req.query.sLon;

    let eLat = req.query.eLat;
    let eLon = req.query.eLon;

    let delta = req.query.dlt;
    // console.log(sLat + " " + sLon + " " + eLat + " " + eLon);

    let elm = GPXdataElements.trkPathBtwnToJSON(path.join(__dirname+'/uploads/')+file, sLat, sLon, eLat, eLon, delta);
    // console.log("elm tr" + elm);
    // let parsed = JSON.parse(elm);
    res.send(elm)

});

app.get('/createGPX', function(req, res){
    console.log("in create");
    let fileName = req.query.file;
    let vers_creator = req.query.both;
    let back = GPXdataElements.createFile(path.join(__dirname+'/uploads/')+fileName, vers_creator);
    let stat = "";
    if(back == 1){
      stat = "1";
    }else{
      stat = "0";
    }
    res.send(stat);

});

app.get('/addRt', function(req, res){
    console.log("in add Rt");
    let fileName = req.query.file;
    let wptStr = req.query.wptStr;
    let rtStr = req.query.rtStr;
    let numWpt = req.query.numWpt;

    let back = GPXdataElements.addRt(path.join(__dirname+'/uploads/')+fileName, wptStr, rtStr, numWpt);
    let stat = "";
    if(back == 1){
      stat = "1";
    }else{
      stat = "0";
    }
    res.send(stat);
});

app.get('/renameCmp', function(req, res){
  console.log("in rename");
  let fileName = req.query.file;
  let newName = req.query.newName;
  let typeNum = req.query.typeNum;
  let type = req.query.type;

  // console.log(fileName + " " + newName + " " + typeNum + " " + type);

  let back = GPXdataElements.renameCmp(path.join(__dirname+'/uploads/')+fileName, newName, typeNum, type);
  let stat = "";
    if(back == 1){
      stat = "1";
    }else{
      stat = "0";
    }
    res.send(stat);
});

let uP = "";
let uN = "";
let dbName = "";
let host = "";
//A4 SQL

app.get('/dbLogin', async function(req, res){
    uP = req.query.uP;
    uN = req.query.uN;
    dbName = req.query.dbName;
    host = req.query.host;

    let result = true;
    let connection;
    try {
        connection = await mysql.createConnection({
            host: host,
            user: uN,
            password: uP,
            database: dbName
        });

        await connection.execute("CREATE TABLE IF NOT EXISTS FILE (gpx_id INT AUTO_INCREMENT, file_name VARCHAR(60) NOT NULL, ver DECIMAL(2,1) NOT NULL, creator VARCHAR(256) NOT NULL, primary key(gpx_id) )");

        await connection.execute("create table if not exists ROUTE (route_id INT AUTO_INCREMENT, route_name VARCHAR(256), route_len FLOAT(15,7) NOT NULL, gpx_id INT NOT NULL, primary key(route_id), FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE)");

        await connection.execute("create table if not exists POINT (point_id INT AUTO_INCREMENT, point_index INT NOT NULL, latitude DECIMAL(11,7) NOT NULL, longitude DECIMAL(11,7) NOT NULL, point_name VARCHAR(256), route_id INT NOT NULL, primary key(point_id), FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE)")

    }catch(e){
        console.log("error " + e);
        result = false;
    }finally{
        if (connection && connection.end) connection.end();
        res.send(result);
    }

});

app.get('/storeFiles', async function(req, res){
    let files = req.query.files;
    let insertFailed = 0;
    // console.log(files);
    let connection;
    try{
        connection = await mysql.createConnection({
            host: host,
            user: uN,
            password: uP,
            database: dbName
        });

        await connection.execute("DELETE FROM FILE");
        for(let i = 0; i < files.length; i++){
            let insertRes = await insertFileTable(files[i]);
            if(insertRes == 0){
                console.log("counld't insert file " + files[i]);
                insertFailed ++;
            }
        }

        // connection.execute("SELECT * FROM FILE;", function (err, rows, fields){
        //     if(err){
        //         console.log("no");
        //     }
        //     for (let row of rows){
        //         console.log("ID: "+row.gpx_id+" Last name: "+row.file_name+" First name: "+row.ver+" mark: "+row.creator );
        //     }
        // });

        await connection.execute("DELETE FROM ROUTE");
        let result = await insertRtTable();
        if(result == 0 ){
            insertFailed++;
        }

        await connection.execute("DELETE FROM POINT");
        result = await insertPtTable();
        if(result == 0 ){
            insertFailed++;
        }

    }catch(e){
        console.log("Query error: "+e);
    }finally{
        if (connection && connection.end) connection.end();
        let result = true;
        if(insertFailed > 0){
            result = false;
        }
        res.send(result);
    }

});

app.get('/update', async function(req, res){
    let files = req.query.files;
    let insertFailed = 0;
    // console.log(files);
    let connection;
    try{
        connection = await mysql.createConnection({
            host: host,
            user: uN,
            password: uP,
            database: dbName
        });


        await connection.execute("DELETE FROM ROUTE");
        let result = await insertRtTable();
        if(result == 0 ){
            insertFailed++;
        }

        await connection.execute("DELETE FROM POINT");
        result = await insertPtTable();
        if(result == 0 ){
            insertFailed++;
        }

    }catch(e){
        console.log("Query update: "+e);
    }finally{
        if (connection && connection.end) connection.end();
        let result = true;
        if(insertFailed > 0){
            result = false;
        }
        res.send(result);
    }

});


// function getdbStatus(numFiles, numRts, numPts){
//     return "DataBase has " + numFiles + "files and " + numRts + "routes and " + numPts + "points";
// }

async function insertFileTable(fileName){
    let row = GPXdataElements.getGPX_JSON(path.join(__dirname+'/uploads/')+fileName);
    let elem = JSON.parse(row);

    let title = "(file_name, ver, creator)";
    let values = "('" + fileName +"', '" + elem.version + "', '" + elem.creator + "')";

    let insert = "INSERT INTO  FILE " + title + "VALUES " + values + ";";
    // console.log("file \n" + insert );
    let ret = 1;
    let connection;
    try{
        connection = await mysql.createConnection({
            host: host,
            user: uN,
            password: uP,
            database: dbName
        });

        await connection.execute(insert);

    }catch(e){
        ret = 0;
        console.log("insertFile " + e);
    }finally{
        return ret;
    }

}

async function insertRtTable(){

    let connection;
    try{

        connection = await mysql.createConnection({
            host: host,
            user: uN,
            password: uP,
            database: dbName
        });


        const [rows,fields] = await connection.query("SELECT * FROM FILE;");

        for(let row of rows){
            let rt = GPXdataElements.getJSON_rtList(path.join(__dirname+'/uploads/')+row.file_name);
            let rtListParse = JSON.parse(rt);

            for(let i = 0; i< rtListParse.length; i++){

                let len = rtListParse[i].len;
                let gpxId = row.gpx_id;

                let name = '';
                if(rtListParse[i].name == "None"){
                    name = null;
                    let title = "( route_len, gpx_id)"
                    let value = "('"+ len + "','" + gpxId + "')";
                    let insert = "INSERT INTO ROUTE " + title + " VALUES " + value + ";";
                    // console.log(insert);

                    await connection.execute(insert);
                }else{
                    name = rtListParse[i].name;
                    let title = "(route_name, route_len, gpx_id)"
                    let value = "('" + name + "', '" + len + "','" + gpxId + "')";
                    let insert = "INSERT INTO ROUTE " + title + " VALUES " + value + ";";
                    // console.log(insert);

                    await connection.execute(insert);
                }


            }
        }
    }catch(e){
        console.log("rtTable " + e);
        return 0;
    }

    return 1;
}

async function insertPtTable(){
    let connection;
    try{
        connection = await mysql.createConnection({
            host: host,
            user: uN,
            password: uP,
            database: dbName
        });

        const [rows,fields] = await connection.query("SELECT FILE.file_name, ROUTE.route_id FROM FILE, ROUTE WHERE ROUTE.gpx_id=FILE.gpx_id order by ROUTE.route_id ASC;");

        let prevFile = "";
        for(let j = 0; j< rows.length ; j++){

            while(j < rows.length && prevFile == rows[j].file_name){
                j++;
            }

            if(j >= rows.length){
                break;
            }

            prevFile = rows[j].file_name;
            let wpt = GPXdataElements.rtPtsToJSON(path.join(__dirname+'/uploads/')+rows[j].file_name);
            let parseWpt = JSON.parse(wpt);

            let alreadyZ = -1;
            for(let i = 0; i< parseWpt.length; i++){


                let lat = parseWpt[i].lat;
                let lon = parseWpt[i].lon;
                let pointIdx = parseWpt[i].pointNum;
                if(pointIdx == 0){
                    alreadyZ++;
                }

                let routeId = rows[j].route_id + alreadyZ;

                let name = '';
                if(parseWpt[i].name == "None"){
                    // name = null;
                    let title = "(point_index, latitude, longitude, route_id)"
                    let value = "('" + pointIdx + "', '" + lat + "','" + lon + "', '"+ routeId + "')";
                    let insert = "INSERT INTO POINT" + title + " VALUES " + value + ";";
                    await connection.execute(insert);
                }else{
                    name = parseWpt[i].name;
                    let title = "(point_index, latitude, longitude, point_name, route_id)"
                    let value = "('" + pointIdx + "', '" + lat + "','" + lon + "', '"+ name + "', '" + routeId + "')";
                    let insert = "INSERT INTO POINT" + title + " VALUES " + value + ";";
                    await connection.execute(insert);
                }

                // console.log(insert);

            }

            // console.log(wpt + "\n");
        }
    }catch(e){
        console.log("pt table " + e);
        return 0;
    }

    return 1;
}

app.get('/dbClear', async function(req, res){

    let result = true;
    let connection;
    try{

        connection = await mysql.createConnection({
            host: host,
            user: uN,
            password: uP,
            database: dbName
        });

        await connection.execute("DELETE FROM POINT;");

        await connection.execute("DELETE FROM ROUTE;");

        await connection.execute("DELETE FROM FILE;");

    }catch(e){
        console.log("clear " + e);
        result = false;
    }finally{
        if (connection && connection.end) connection.end();

        res.send(result);
    }
});


app.get('/dbStatus', async function(req, res){
    let result = true;
    let numFiles;
    let numRts;
    let numPts;
    let connection;

    try{

        connection = await mysql.createConnection({
            host: host,
            user: uN,
            password: uP,
            database: dbName
        });

        const [rows1,fields1] = await connection.execute("SELECT COUNT(*) AS numFiles FROM FILE;");
        for(let row of rows1){
            numFiles = row.numFiles;
        }


        const [rows2, fields2] = await connection.execute("SELECT COUNT(*) AS numRts FROM ROUTE;");
        for(let row of rows2){
            numRts = row.numRts;
        }

        const [rows3, fields3] = await connection.execute("SELECT COUNT(*) AS numPts FROM POINT;");
        for(let row of rows3){
            numPts = row.numPts;
        }

    }catch(e){
        console.log("dbStatus " + e);
        result = false;
    }finally{
        if (connection && connection.end) connection.end();

        res.send({
            numFiles: numFiles,
            numRts: numRts,
            numPts: numPts,
            result: result
        });
    }
});

//q1
app.get('/allRts', async function(req, res){
    let rtsList = [];
    let sortType = req.query.sortVal;
    let result = true;
    let connection;

    try{
        connection = await mysql.createConnection({
            host: host,
            user: uN,
            password: uP,
            database: dbName
        });

        if(sortType == "Length"){
            const [rows, fields] = await connection.execute("SELECT * FROM ROUTE ORDER BY route_len;");
            for(let row of rows){
                let rtName = row.route_name;
                let len = row.route_len;
                if(rtName == null){
                    rtName = "Unnamed Route";
                }
                let obj = {name:rtName,len:len};
                rtsList.push(obj);
            }
        }else{
            const [rows, fields] = await connection.execute("SELECT * FROM ROUTE ORDER BY route_name;");
            for(let row of rows){
                let rtName = row.route_name;
                let len = row.route_len;
                if(rtName == null){
                    rtName = "Unnamed Route";
                }
                let obj = {name:rtName,len:len};
                rtsList.push(obj);
            }
        }
    }catch(e){
        result = false;
        console.log("q1 " + e);
    }finally{
        // console.log(rtsList);
        // let list = JSON.parse(JSON.stringify(rtsList));
        // console.log(list);
        if (connection && connection.end) connection.end();

        let list = JSON.stringify(rtsList);
        res.send({
            rtsList: list,
            result: result
        });
    }
});

//q2
app.get('/fileRts', async function(req, res){

    let rtsList = [];
    let sortType = req.query.sortVal;
    let result = true;

    let connection;

    try{
        connection = await mysql.createConnection({
            host: host,
            user: uN,
            password: uP,
            database: dbName
        });
        // select * from ROUTE,FILE where ROUTE.gpx_id=FILE.gpx_id AND FILE.file_name="GHTC_James-Stone(2rt_0trk_0seg_4220m).gpx";
        let fileName= req.query.file;
        if(sortType == "Length"){

            const [rows, fields] = await connection.execute("select * from ROUTE,FILE where ROUTE.gpx_id=FILE.gpx_id AND FILE.file_name=" + '"' + fileName + '"' + " order by ROUTE.route_len;" );
            for(let row of rows){
                let rtName = row.route_name;
                let len = row.route_len;

                if(rtName == null){
                    rtName = "Unnamed Route";
                }
                let obj = {name:rtName,len:len};
                rtsList.push(obj);
            }
        }else{
            const [rows, fields] = await connection.execute("select * from ROUTE,FILE where ROUTE.gpx_id=FILE.gpx_id AND FILE.file_name=" + '"' + fileName + '"' + " order by ROUTE.route_name;");
            for(let row of rows){
                let rtName = row.route_name;
                let len = row.route_len;

                if(rtName == null){
                    rtName = "Unnamed Route";
                }
                let obj = {name:rtName,len:len};
                rtsList.push(obj);
            }
        }
    }catch(e){
        result = false;
        console.log("q2 " + e);
    }finally{
        // console.log(rtsList);
        // let list = JSON.parse(JSON.stringify(rtsList));
        // console.log(list);
        if (connection && connection.end) connection.end();

        let list = JSON.stringify(rtsList);
        res.send({
            rtsList: list,
            result: result
        });
    }

});

//q4
app.get('/q4', async function(req,res){

    let rtsList = [];
    let sortType = req.query.sortVal;
    let result = true;

    let connection;

    try{
        // select * from FILE join ROUTE on FILE.gpx_id=ROUTE.gpx_id join POINT on ROUTE.route_id=POINT.route_id and FILE.file_name="simple.gpx" order by ROUTE.route_len, ROUTE.route_id, POINT.point_index ASC;
        connection = await mysql.createConnection({
            host: host,
            user: uN,
            password: uP,
            database: dbName
        });

        let fileName= req.query.file;
        if(sortType == "Route Length"){

            const [rows, fields] = await connection.execute("select * from FILE join ROUTE on FILE.gpx_id=ROUTE.gpx_id join POINT on ROUTE.route_id=POINT.route_id and FILE.file_name=" + '"' + fileName + '"' + " order by ROUTE.route_len, ROUTE.route_id, POINT.point_index ASC;" );
            let prevRtId = "";
            let rtNum = 0;
            for(let row of rows){
                let rtName = row.route_name;
                let lat = row.latitude;
                let lon = row.longitude;

                let rtId = row.route_id;
                if(prevRtId == rtId){
                    rtNum++;
                }else{
                    prevRtId = row.route_id;
                    rtNum = 0;
                }

                if(rtName == null){
                    let num = rtNum + 1;
                    rtName = "Unnamed Route " + num;
                }

                let ptIdx = row.point_index;

                let ptName = row.point_name;
                if(ptName == null){
                    ptName = "Unnamed Point " + ptIdx;
                }

                let rtLen = row.route_len;

                let obj = {ptIdx:ptIdx,rtName:rtName,rtLen:rtLen,ptName:ptName,lat:lat,lon:lon};
                rtsList.push(obj);
            }
        }else{
            const [rows, fields] = await connection.execute("select * from FILE join ROUTE on FILE.gpx_id=ROUTE.gpx_id join POINT on ROUTE.route_id=POINT.route_id and FILE.file_name=" + '"' + fileName + '"' + " order by ROUTE.route_name, ROUTE.route_id, POINT.point_index ASC;");
            let prevRtId = "";
            let rtNum = 0;
            for(let row of rows){
                let rtName = row.route_name;
                let lat = row.latitude;
                let lon = row.longitude;

                let rtId = row.route_id;
                if(prevRtId == rtId){
                    rtNum++;
                }else{
                    prevRtId = row.route_id;
                    rtNum = 0;
                }

                if(rtName == null){
                    let num = rtNum + 1;
                    rtName = "Unnamed Route " + num;
                }

                let ptIdx = row.point_index;

                let ptName = row.point_name;
                if(ptName == null){
                    ptName = "Unnamed Point " + ptIdx;
                }

                let rtLen = row.route_len;

                let obj = {ptIdx:ptIdx,rtName:rtName,rtLen:rtLen,ptName:ptName,lat:lat,lon:lon};
                rtsList.push(obj);
            }
        }
    }catch(e){
        result = false;
        console.log("q4 " + e);
    }finally{
        // console.log(rtsList);
        // let list = JSON.parse(JSON.stringify(rtsList));
        // console.log(list);
        if (connection && connection.end) connection.end();

        let list = JSON.stringify(rtsList);
        res.send({
            rtsList: list,
            result: result
        });
    }

});

//q5
app.get('/q5', async function(req,res){

    let rtsList = [];
    let sortType = req.query.sortVal;
    let numRows = req.query.numRows;
    let result = true;
    let tableSortType = req.query.sortVal2;
    let connection;

    try{
        // select X.route_name, X.route_len from (select route_name, route_len from ROUTE,FILE where ROUTE.gpx_id=FILE.gpx_id and FILE.file_name="simple.gpx" order by ROUTE.route_len DESC limit 5)X order by X.route_len;
        connection = await mysql.createConnection({
            host: host,
            user: uN,
            password: uP,
            database: dbName
        });

        let fileName= req.query.file;
        if(sortType == "Longest"){


            if( tableSortType == "Route Length"){
                const [rows,fields] = await connection.execute("select X.route_name, X.route_len from (select route_name, route_len from ROUTE,FILE where ROUTE.gpx_id=FILE.gpx_id and FILE.file_name=" + '"' + fileName + '"' + " order by ROUTE.route_len DESC limit " + numRows + " )X order by X.route_len;" );

                for(let row of rows){
                    let rtName = row.route_name;

                    if(rtName == null){
                        rtName = "Unnamed Route";
                    }
                    let rtLen = row.route_len;

                    let obj = {rtName:rtName,rtLen:rtLen};
                    rtsList.push(obj);
                }
            }else{
                const [rows,fields] = await connection.execute("select X.route_name, X.route_len from (select route_name, route_len from ROUTE,FILE where ROUTE.gpx_id=FILE.gpx_id and FILE.file_name=" + '"' + fileName + '"' + " order by ROUTE.route_len DESC limit " + numRows + " )X order by X.route_name;" );


                for(let row of rows){
                    let rtName = row.route_name;

                    if(rtName == null){
                        rtName = "Unnamed Route";
                    }

                    let rtLen = row.route_len;

                    let obj = {rtName:rtName,rtLen:rtLen};
                    rtsList.push(obj);
                }
            }

        }else{

            if( tableSortType == "Route Length"){
                const [rows,fields] = await connection.execute("select X.route_name, X.route_len from (select route_name, route_len from ROUTE,FILE where ROUTE.gpx_id=FILE.gpx_id and FILE.file_name=" + '"' + fileName + '"' + " order by ROUTE.route_len ASC limit " + numRows + " )X order by X.route_len;" );

                for(let row of rows){
                    let rtName = row.route_name;

                    if(rtName == null){
                        rtName = "Unnamed Route";
                    }

                    let rtLen = row.route_len;

                    let obj = {rtName:rtName,rtLen:rtLen};
                    rtsList.push(obj);
                }
            }else{
                const [rows,fields] = await connection.execute("select X.route_name, X.route_len from (select route_name, route_len from ROUTE,FILE where ROUTE.gpx_id=FILE.gpx_id and FILE.file_name=" + '"' + fileName + '"' + " order by ROUTE.route_len ASC limit " + numRows + " )X order by X.route_name;" );


                for(let row of rows){
                    let rtName = row.route_name;

                    if(rtName == null){
                        rtName = "Unnamed Route";
                    }

                    let rtLen = row.route_len;

                    let obj = {rtName:rtName,rtLen:rtLen};
                    rtsList.push(obj);
                }
            }

        }
    }catch(e){
        result = false;
        console.log("q5 " + e);
    }finally{
        // console.log(rtsList);
        // let list = JSON.parse(JSON.stringify(rtsList));
        if (connection && connection.end) connection.end();

        let list = JSON.stringify(rtsList);
        res.send({
            rtsList: list,
            result: result
        });
    }

});

app.get('/q3', async function(req, res){

    // select * from FILE join ROUTE on FILE.gpx_id=ROUTE.gpx_id and FILE.file_name="simple.gpx";
    // select * from POINT where route_id=127;

    let fileName = req.query.file;
    let rtNum = req.query.rtNum;
    let rtsList = [];
    let result = true;
    let rtName = "";
    let rtId = 0;

    let connection;

    try{

        connection = await mysql.createConnection({
            host: host,
            user: uN,
            password: uP,
            database: dbName
        });

        const [rows,fields] = await connection.execute(" select * from FILE join ROUTE on FILE.gpx_id=ROUTE.gpx_id and FILE.file_name=" + '"' + fileName + '"' + " order by ROUTE.route_id ASC;" );
        let i = 0;
        for(let row of rows){
            rtName = row.route_name;
            rtId = row.route_id;
            if(rtName == null){
                rtName = "Unnamed Route";
            }
            if(i == rtNum-1){
                break;
            }else{
                i++;
            }
        }

        const [rows2,fields2] = await connection.execute(" select * from POINT where route_id=" + rtId + " order by point_index ASC;" );

        for(let row of rows2){
            let ptName = row.point_name;
            if(ptName == null){
                ptName = "Unnamed Point";
            }

            let lat = row.latitude;
            let lon = row.longitude;

            let ptIdx = row.point_index;

            let obj = {ptName:ptName,lat:lat,lon:lon,ptIdx:ptIdx};
            rtsList.push(obj);

        }


    }catch(e){
        result = false;
        console.log("q3 " + e);
    }finally{
        if (connection && connection.end) connection.end();

        let list = JSON.stringify(rtsList);
        res.send({
            rtsList: list,
            result: result,
            rtName: rtName,
        });
    }

});


app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
