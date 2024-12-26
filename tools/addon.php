<?php
//cmd:  php addon.php -nXXXX 
$argc = $_SERVER['argc'];
$addonName = "";
if($argc>1){
	$argv = $_SERVER['argv'];
	for($i=1;$i<$argc;$i++){
		$param = $argv[$i];
		if(substr($param,0,2)=='-n'){
			$addonName = substr($param,2);
			
		}
	}
}else{
	echo "Are you sure you want to create addon project?  Type the name of addon:";
	$handle = fopen ("php://stdin","r");
	$addonName = fgets($handle);
	$addonName = str_replace(array(chr(13),chr(10)),'',$addonName);
	fclose($handle);
}
$addonName = trim($addonName);
if(!$addonName){
	echo "Addon name invalid!";
	exit;
}

$rootDir = realpath(dirname(__FILE__)."/../addon");




function listDir($srcDir,$dstDir,$addonName) {    
	$mydir=opendir($srcDir);    
	//echo "<ul>";    
	while(($file=readdir($mydir))!==false) {        
		if ($file!='.'&&$file!='..') {            
			if (is_dir($file)) {                
				//echo "<li class='dir'>$file</li>";                
				listDir("$srcDir/$file","$dstDir/$file",$addonName);            
			}else{                
				//echo "<li class='file'><a href='$srcDir/$file' target='_blank'>$file</a></li>";
				$filename = str_replace(array("{tpl}","{TPL}"),array(strtolower($addonName),strtoupper($addonName)),$file);
				$content = file_get_contents($srcDir."/".$file);
				$content = str_replace(array("{tpl}","{TPL}"),array(strtolower($addonName),strtoupper($addonName)),$content);
				file_put_contents($dstDir.'/'.$filename,$content);
			}
		}
   }
   closedir($mydir);    
  //echo "</ul>";
}
 
$dstDir = ($rootDir.DIRECTORY_SEPARATOR.$addonName);

if(is_dir($dstDir)){
	echo "Addon create failed,{$dstDir} is exists";exit;
}else{
	mkdir($dstDir);
}

listDir($rootDir.DIRECTORY_SEPARATOR.'Template',$dstDir,$addonName);


echo "Addon create successfully!";

?>