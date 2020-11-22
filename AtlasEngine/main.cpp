#include "globals.h"

struct Global global;

int main()
{
    printf("Globals: %d bytes\n", sizeof(global));

    // this should be the one and only "malloc"
    void* mempackRamData = malloc(ramSize);

    // initialize mempack for RAM
    MEMPACK_Init(&global.ramPack, mempackRamData, ramSize, "ram");

    // create a window, create context,
    // create GPU Mempacks, etc
    InitOpenGL();

    Sprite* manSprite = Sprite_Init("Assets/man.bmp");
    Sprite* coinSprite = Sprite_Init("Assets/coin.bmp");

    printf("Allocating instances\n");

    Instance* manInst1 = (Instance*)MEMPACK_AllocMem(&global.ramPack, sizeof(Instance), "manInst");
    manInst1->sprite = manSprite;

    // draw person on top (small depth)
    manInst1->depth = 0.1f;
    manInst1->posX = -0.5f;
    manInst1->posY = -0.5f;
    manInst1->scaleX = 0.00075f;
    manInst1->scaleY = 0.00075f;

    Instance* coinInst[5];
    
    for (int i = 0; i < 5; i++)
    {
        coinInst[i] = (Instance*)MEMPACK_AllocMem(&global.ramPack, sizeof(Instance), "coinInst");
        coinInst[i]->sprite = coinSprite;

        // draw coins on bottom (larger depth)
        coinInst[i]->depth = 0.2f; 
        coinInst[i]->posX = 0.5f;
        coinInst[i]->posY = -0.5f + 0.2f * i;
        coinInst[i]->scaleX = 0.002f;
        coinInst[i]->scaleY = 0.002f;
    }

    // timer variables
    clock_t start;
    clock_t end;

    // initialize time
    start = clock();


    // render loop
    // -----------
    while (!glfwWindowShouldClose(global.window))
    {
        // measure elapsed time per frame
        end = clock();
        float elapsedTime = (end - start) / 1000.0f;
        start = clock();

        // input
        // -----
        if (glfwGetKey(global.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(global.window, true);

        if (glfwGetKey(global.window, GLFW_KEY_LEFT) == GLFW_PRESS)
            manInst1->posX -= elapsedTime;

        if (glfwGetKey(global.window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            manInst1->posX += elapsedTime;

        if (glfwGetKey(global.window, GLFW_KEY_UP) == GLFW_PRESS)
            manInst1->posY += elapsedTime;

        if (glfwGetKey(global.window, GLFW_KEY_DOWN) == GLFW_PRESS)
            manInst1->posY -= elapsedTime;

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Instance_Draw(manInst1);

        for (int i = 0; i < 5; i++)
        {
            Instance_Draw(coinInst[i]);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(global.window);
        glfwPollEvents();
    }

    // erase all allocated data
    MEMPACK_Clean(&global.ramPack);

    // erase resources
    delete mempackRamData;
    mempackRamData = nullptr;

    CleanOpenGL();

    // end program
    return 0;
}