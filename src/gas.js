function doPost(e) {
    var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName('raw');
    var nextRaw = sheet.getLastRow() + 1;
    var data = e.postData.getDataAsString().split('\n');
  
    for(var d of data){
       sheet.getRange(nextRaw, 1).setValue(d);
       nextRaw++;
    }
}