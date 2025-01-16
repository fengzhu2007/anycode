// 定义一个类
class Demo {
    // 显示信息的方法
    displayInfo() {
        console.log(`Name: ${this.name}, Number: ${this.number}`);
    }
}

while(true){

}


const mockGetOSSData = ()=>{
    return new Promise(function(resolve, reject) {
        axios.get("/api/oss/signature",{'dir':'file'}).then((response)=>{
            if(USER_ASSERT(response.data)){
                resolve(response.data.data);

            }else{
                reject(null);
            }
        }).finally(()=>{

        })
    })
};

var demo = new Demo;

console.log(`name:${demo.name};age:${demo.age}`);
