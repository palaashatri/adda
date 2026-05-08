plugins {
    id("org.springframework.boot")
    id("io.spring.dependency-management")
}

dependencies {
    implementation(project(":backend:shared"))
    implementation("org.springframework.boot:spring-boot-starter-websocket")
    implementation("com.google.protobuf:protobuf-java:3.25.3")
    implementation("com.uber:h3:4.1.1")
    implementation("com.fasterxml.jackson.core:jackson-databind")
    
    testImplementation("org.springframework.boot:spring-boot-starter-test")
}
