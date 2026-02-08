#include "../Header/Util.h";

#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "../Header/stb_image.h"


int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

unsigned int compileShader(GLenum type, const char* source)
{
    //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
    //Citanje izvornog koda iz fajla
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str(); //Izvorni kod sejdera koji citamo iz fajla na putanji "source"

    int shader = glCreateShader(type); //Napravimo prazan sejder odredjenog tipa (vertex ili fragment)

    int success; //Da li je kompajliranje bilo uspesno (1 - da)
    char infoLog[512]; //Poruka o gresci (Objasnjava sta je puklo unutar sejdera)
    glShaderSource(shader, 1, &sourceCode, NULL); //Postavi izvorni kod sejdera
    glCompileShader(shader); //Kompajliraj sejder

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); //Proveri da li je sejder uspesno kompajliran
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog); //Pribavi poruku o gresci
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    //Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource

    unsigned int program; //Objedinjeni sejder
    unsigned int vertexShader; //Verteks sejder (za prostorne podatke)
    unsigned int fragmentShader; //Fragment sejder (za boje, teksture itd)

    program = glCreateProgram(); //Napravi prazan objedinjeni sejder program

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource); //Napravi i kompajliraj vertex sejder
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource); //Napravi i kompajliraj fragment sejder

    //Zakaci verteks i fragment sejdere za objedinjeni program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program); //Povezi ih u jedan objedinjeni sejder program
    glValidateProgram(program); //Izvrsi proveru novopecenog programa

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success); //Slicno kao za sejdere
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    //Posto su kodovi sejdera u objedinjenom sejderu, oni pojedinacni programi nam ne trebaju, pa ih brisemo zarad ustede na memoriji
    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}

unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Proverava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 2: InternalFormat = GL_RG; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}

GLFWcursor* loadImageToCursor(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;

    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);

    if (ImageData != NULL)
    {
        GLFWimage image;
        image.width = TextureWidth;
        image.height = TextureHeight;
        image.pixels = ImageData;

        // Tacka na površini slike kursora koja se ponaša kao hitboks, moze se menjati po potrebi
        // Trenutno je gornji levi ugao, odnosno na 20% visine i 20% sirine slike kursora
        int hotspotX = TextureWidth / 5;
        int hotspotY = TextureHeight / 5;

        GLFWcursor* cursor = glfwCreateCursor(&image, hotspotX, hotspotY);
        stbi_image_free(ImageData);
        return cursor;
    }
    else {
        std::cout << "Kursor nije ucitan! Putanja kursora: " << filePath << std::endl;
        stbi_image_free(ImageData);

    }
}

Model loadOBJModel(const char* filePath) {
    Model model;
    std::vector<float> vertices;
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        std::cout << "Ne mogu otvoriti OBJ fajl: " << filePath << std::endl;
        return model;
    }
    
    std::vector<float> tempPositions;
    std::vector<float> tempNormals;
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "v") {
            // Vertex pozicija
            float x, y, z;
            iss >> x >> y >> z;
            tempPositions.push_back(x);
            tempPositions.push_back(y);
            tempPositions.push_back(z);
        }
        else if (prefix == "vn") {
            // Vertex normala
            float nx, ny, nz;
            iss >> nx >> ny >> nz;
            tempNormals.push_back(nx);
            tempNormals.push_back(ny);
            tempNormals.push_back(nz);
        }
        else if (prefix == "f") {
            // Face (trougao)
            std::string vertex1, vertex2, vertex3;
            iss >> vertex1 >> vertex2 >> vertex3;
            
            // Parsiranje formata v/vt/vn ili v//vn
            for (const std::string& vertexStr : {vertex1, vertex2, vertex3}) {
                int posIndex = -1, normalIndex = -1;
                
                size_t slash1 = vertexStr.find('/');
                if (slash1 != std::string::npos) {
                    posIndex = std::stoi(vertexStr.substr(0, slash1)) - 1;
                    size_t slash2 = vertexStr.find('/', slash1 + 1);
                    if (slash2 != std::string::npos && slash2 > slash1 + 1) {
                        normalIndex = std::stoi(vertexStr.substr(slash2 + 1)) - 1;
                    }
                    else if (slash2 != std::string::npos) {
                        // Format v//vn
                        normalIndex = std::stoi(vertexStr.substr(slash2 + 1)) - 1;
                    }
                } else {
                    posIndex = std::stoi(vertexStr) - 1;
                }
                
                // Dodaj poziciju
                if (posIndex >= 0 && posIndex * 3 + 2 < (int)tempPositions.size()) {
                    vertices.push_back(tempPositions[posIndex * 3]);
                    vertices.push_back(tempPositions[posIndex * 3 + 1]);
                    vertices.push_back(tempPositions[posIndex * 3 + 2]);
                }
                
                // Dodaj normalu
                if (normalIndex >= 0 && normalIndex * 3 + 2 < (int)tempNormals.size()) {
                    vertices.push_back(tempNormals[normalIndex * 3]);
                    vertices.push_back(tempNormals[normalIndex * 3 + 1]);
                    vertices.push_back(tempNormals[normalIndex * 3 + 2]);
                } else {
                    // Ako nema normale, koristi default
                    vertices.push_back(0.0f);
                    vertices.push_back(1.0f);
                    vertices.push_back(0.0f);
                }
            }
        }
    }
    
    file.close();
    
    if (vertices.empty()) {
        std::cout << "OBJ fajl je prazan ili neispravan: " << filePath << std::endl;
        return model;
    }
    
    // Kreiraj VAO i VBO
    glGenVertexArrays(1, &model.VAO);
    glGenBuffers(1, &model.VBO);
    
    glBindVertexArray(model.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, model.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Pozicija atribut (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normala atribut (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    model.vertexCount = vertices.size() / 6;  // 6 floatova po verteksu (3 pozicija + 3 normala)
    
    std::cout << "OBJ model ucitan uspesno: " << filePath << " (" << model.vertexCount << " verteksa)" << std::endl;
    
    return model;
}