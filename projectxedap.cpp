/**************************************
 * File: cycle_with_person.c
 * Description: Hierarchical 3D Model of a Bicycle with a Person, W,A,S,D, +/- controls, Q for wheelie, with on-screen control labels
 **************************************
 
 **************************************************************************
 *                          PROJECT INFORMATION                           *
 **************************************************************************
 * File Name    : projectxadep.cpp
 * Author       : Hoang Minh Tri - Nguyen Huu Phong
 * Edited Date  : 17/04/2025
 * Description  : Du an xe dap qua la dep trai cua hmtri va phong:))
 **************************************************************************/

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PI              3.141592653589793
#define WIN_WIDTH       1200
#define WIN_HEIGHT      600
#define CYCLE_LENGTH    3.3f
#define ROD_RADIUS      0.05f
#define NUM_SPOKES      20
#define SPOKE_ANGLE     (360.0f / NUM_SPOKES)
#define RADIUS_WHEEL    1.0f
#define TUBE_WIDTH      0.08f
#define RIGHT_ROD       1.6f
#define RIGHT_ANGLE     56.0f
#define MIDDLE_ROD      1.7f
#define MIDDLE_ANGLE    116.0f
#define BACK_CONNECTOR  0.5f
#define LEFT_ANGLE      58.0f
#define WHEEL_OFFSET    0.11f
#define WHEEL_LEN       1.1f
#define TOP_LEN         1.5f
#define CRANK_ROD       0.7f
#define CRANK_RODS      1.12f
#define CRANK_ANGLE     12.0f
#define HANDLE_ROD      1.2f
#define FRONT_INCLINE   75.0f
#define HANDLE_LIMIT    70.0f
#define INC_STEERING    2.0f
#define INC_SPEED       0.2f
#define MAX_SPEED       0.2f
#define MIN_SPEED      -0.2f
#define WHEELIE_ANGLE   30.0f // Maximum angle for wheelie
#define WHEELIE_DURATION 1000 // Duration in milliseconds (1 second)

/*****************************************
 * Global Variables
 ****************************************/
GLfloat pedalAngle, speed, steering;
GLfloat camx, camy, camz;
GLfloat anglex, angley, anglez;
int prevx, prevy;
GLenum Mouse;
GLfloat xpos, zpos, direction;
GLfloat wheelieAngle = 0.0f; // Angle for wheelie effect
int wheelieActive = 0; // Flag to check if wheelie is active
int wheelieTimer = 0; // Timer to control wheelie duration

// Prototype functions
void ZCylinder(GLfloat radius, GLfloat length);
void XCylinder(GLfloat radius, GLfloat length);
void drawFrame(void);
void gear(GLfloat inner_radius, GLfloat outer_radius,
          GLfloat width, GLint teeth, GLfloat tooth_depth);
void drawChain(void);
void drawPedals(void);
void drawTyre(void);
void drawSeat(void);
void drawPerson(void);
void drawControlsText(void); // New function for on-screen controls
void help(void);
void init(void);
void reset(void);
void display(void);
void idle(void);
void updateScene(void);
void landmarks(void);
void special(int key, int x, int y);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void reshape(int w, int h);
void glSetupFuncs(void);
GLfloat Abs(GLfloat);
GLfloat degrees(GLfloat);
GLfloat radians(GLfloat);
GLfloat angleSum(GLfloat, GLfloat);
void wheelieReset(int value);

/************************************************
 * Utility function: Draw a cylinder along Z-axis
 ************************************************/
void ZCylinder(GLfloat radius, GLfloat length)
{
    GLUquadricObj *cylinder = gluNewQuadric();
    gluCylinder(cylinder, radius, radius, length, 15, 5);
    gluDeleteQuadric(cylinder);
}

/************************************************
 * Utility function: Draw a cylinder along X-axis
 ************************************************/
void XCylinder(GLfloat radius, GLfloat length)
{
    glPushMatrix();
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    ZCylinder(radius, length);
    glPopMatrix();
}

/*******************************************
 * Update scene: Bicycle movement
 *******************************************/
void updateScene()
{
    GLfloat xDelta, zDelta;
    GLfloat rotation;
    GLfloat sin_steering, cos_steering;
    const GLfloat DECELERATION = 0.02f;

    if (Abs(speed) > 0.0f && Abs(speed) < INC_SPEED / 10.0f) {
        speed = 0.0f;
    } else if (speed != 0.0f) {
        speed -= (speed > 0.0f ? DECELERATION : -DECELERATION);
        if (Abs(speed) < DECELERATION) speed = 0.0f;
    }

    if (Abs(speed) < INC_SPEED / 10.0f) return;

    xDelta = speed * cos(radians(direction + steering));
    zDelta = speed * sin(radians(direction + steering));
    xpos += xDelta;
    zpos -= zDelta;

    pedalAngle = degrees(angleSum(radians(pedalAngle), speed / RADIUS_WHEEL));

    sin_steering = sin(radians(steering));
    cos_steering = cos(radians(steering));
    rotation = atan2(speed * sin_steering, CYCLE_LENGTH + speed * cos_steering);
    direction = degrees(angleSum(radians(direction), rotation));
}

/******************************************
 * angleSum: Return a+b mod 2PI
 ******************************************/
GLfloat angleSum(GLfloat a, GLfloat b)
{
    a += b;
    while (a >= 2 * PI) a -= 2 * PI;
    while (a < 0) a += 2 * PI;
    return a;
}

/************************************************
 * Draw the metal frame of the cycle
 ************************************************/
void drawFrame()
{
    glColor3f(0.4f, 0.0f, 0.0f);

    glPushMatrix();
        // Move to the rear wheel position (pivot point for wheelie)
        glTranslatef(-(BACK_CONNECTOR + RADIUS_WHEEL + TUBE_WIDTH), 0.0f, 0.0f);
        // Apply wheelie rotation around the X-axis at the rear wheel
        glRotatef(wheelieAngle, 1.0f, 0.0f, 0.0f);
        // Translate back to the original position
        glTranslatef((BACK_CONNECTOR + RADIUS_WHEEL + TUBE_WIDTH), 0.0f, 0.0f);

        // Gear and pedal connector
        glPushMatrix();
            glColor3f(0.7f, 0.0f, 0.7f);
            glPushMatrix();
                glTranslatef(0.0f, 0.0f, 0.10f);
                glRotatef(-(pedalAngle + 15.0f), 0.0f, 0.0f, 1.0f);
                gear(0.08f, 0.3f, 0.03f, 30, 0.03f);
            glPopMatrix();
            glColor3f(0.4f, 0.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, -0.25f);
            ZCylinder(0.08f, 0.32f);
        glPopMatrix();

        // Right rod
        glRotatef(RIGHT_ANGLE + 7.0f, 0.0f, 0.0f, 1.0f);
        glScalef(1.0f, 0.8f, 1.0f);
        XCylinder(ROD_RADIUS, RIGHT_ROD);

        // Middle rod
        glRotatef(MIDDLE_ANGLE - (RIGHT_ANGLE + 7.0f) + 5.0f, 0.0f, 0.0f, 1.0f);
        glScalef(0.9f, 1.1f, 1.0f);
        XCylinder(ROD_RADIUS, MIDDLE_ROD);

        // Seat
        glColor3f(0.1f, 1.0f, 0.3f);
        glTranslatef(MIDDLE_ROD, 0.0f, 0.0f);
        glRotatef(-MIDDLE_ANGLE - 7.0f, 0.0f, 0.0f, 1.0f);
        glScalef(0.6f, ROD_RADIUS * 1.7f, 0.4f);
        drawSeat();
        glColor3f(0.4f, 0.0f, 0.0f);
    glPopMatrix();

    // Horizontal connector
    glPushMatrix();
        // Apply wheelie rotation for this part as well
        glTranslatef(-(BACK_CONNECTOR + RADIUS_WHEEL + TUBE_WIDTH), 0.0f, 0.0f);
        glRotatef(wheelieAngle, 1.0f, 0.0f, 0.0f);
        glTranslatef((BACK_CONNECTOR + RADIUS_WHEEL + TUBE_WIDTH), 0.0f, 0.0f);

        glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
        XCylinder(ROD_RADIUS, BACK_CONNECTOR);
        glPushMatrix();
            glTranslatef(0.5f, 0.0f, WHEEL_OFFSET);
            XCylinder(ROD_RADIUS, RADIUS_WHEEL + TUBE_WIDTH);
        glPopMatrix();
        glPushMatrix();
            glTranslatef(0.5f, 0.0f, -WHEEL_OFFSET);
            XCylinder(ROD_RADIUS, RADIUS_WHEEL + TUBE_WIDTH);
        glPopMatrix();
    glPopMatrix();

    // Left rod and wheel
    glPushMatrix();
        // Apply wheelie rotation
        glTranslatef(-(BACK_CONNECTOR + RADIUS_WHEEL + TUBE_WIDTH), 0.0f, 0.0f);
        glRotatef(wheelieAngle, 1.0f, 0.0f, 0.0f);
        glTranslatef((BACK_CONNECTOR + RADIUS_WHEEL + TUBE_WIDTH), 0.0f, 0.0f);

        glTranslatef(-(BACK_CONNECTOR + RADIUS_WHEEL + TUBE_WIDTH), 0.0f, 0.0f);
        glPushMatrix();
            glRotatef(-(2 * pedalAngle + 20.0f), 0.0f, 0.0f, 1.0f);
            drawTyre();
            glColor3f(1.0f, 0.3f, 0.0f);
            gear(0.03f, 0.15f, 0.03f, 20, 0.03f);
            glColor3f(0.4f, 0.0f, 0.0f);
        glPopMatrix();
        glRotatef(LEFT_ANGLE + 10.0f, 0.0f, 0.0f, 1.0f);
        glPushMatrix();
            glTranslatef(0.0f, 0.0f, -WHEEL_OFFSET);
            XCylinder(ROD_RADIUS, WHEEL_LEN);
        glPopMatrix();
        glPushMatrix();
            glTranslatef(0.0f, 0.0f, WHEEL_OFFSET);
            XCylinder(ROD_RADIUS, WHEEL_LEN);
        glPopMatrix();
        glTranslatef(WHEEL_LEN, 0.0f, 0.0f);
        XCylinder(ROD_RADIUS, CRANK_ROD);
        glTranslatef(CRANK_ROD, 0.0f, 0.0f);
        glRotatef(-LEFT_ANGLE - 7.0f, 0.0f, 0.0f, 1.0f);
        XCylinder(ROD_RADIUS, TOP_LEN);
        glTranslatef(TOP_LEN, 0.0f, 0.0f);
        glRotatef(-FRONT_INCLINE - 10.0f, 0.0f, 0.0f, 1.0f);
        glPushMatrix();
            glTranslatef(-0.1f, 0.0f, 0.0f);
            XCylinder(ROD_RADIUS, 0.45f);
        glPopMatrix();
        glPushMatrix();
            glRotatef(-steering * 1.5f, 1.0f, 0.0f, 0.0f);
            glTranslatef(-0.3f, 0.0f, 0.0f);
            glPushMatrix();
                glRotatef(FRONT_INCLINE + 15.0f, 0.0f, 0.0f, 1.0f);
                glPushMatrix();
                    glTranslatef(0.0f, 0.0f, -HANDLE_ROD / 2);
                    ZCylinder(ROD_RADIUS, HANDLE_ROD);
                glPopMatrix();
                glPushMatrix();
                    glColor3f(0.0f, 1.0f, 0.9f);
                    glTranslatef(0.0f, 0.0f, -HANDLE_ROD / 2);
                    ZCylinder(0.07f, HANDLE_ROD / 4);
                    glTranslatef(0.0f, 0.0f, HANDLE_ROD * 3 / 4);
                    ZCylinder(0.07f, HANDLE_ROD / 4);
                    glColor3f(0.4f, 0.0f, 0.0f);
                glPopMatrix();
            glPopMatrix();
            glPushMatrix();
                XCylinder(ROD_RADIUS, CRANK_ROD);
                glTranslatef(CRANK_ROD, 0.0f, 0.0f);
                glRotatef(CRANK_ANGLE + 15.0f, 0.0f, 0.0f, 1.0f);
                glPushMatrix();
                    glTranslatef(0.0f, 0.0f, WHEEL_OFFSET);
                    XCylinder(ROD_RADIUS, CRANK_RODS);
                glPopMatrix();
                glPushMatrix();
                    glTranslatef(0.0f, 0.0f, -WHEEL_OFFSET);
                    XCylinder(ROD_RADIUS, CRANK_RODS);
                glPopMatrix();
                glTranslatef(CRANK_RODS, 0.0f, 0.0f);
                glRotatef(-2 * pedalAngle - 15.0f, 0.0f, 0.0f, 1.0f);
                drawTyre();
            glPopMatrix();
        glPopMatrix();
    glPopMatrix();
}

/********************************************
 * Draw a gear wheel
 ********************************************/
void gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
          GLint teeth, GLfloat tooth_depth)
{
    GLint i;
    GLfloat r0 = inner_radius;
    GLfloat r1 = outer_radius - tooth_depth / 2.0f;
    GLfloat r2 = outer_radius + tooth_depth / 2.0f;
    GLfloat angle, da = 2.0 * PI / teeth / 4.0;

    glShadeModel(GL_FLAT);
    glNormal3f(0.0f, 0.0f, 1.0f);

    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.0 * PI / teeth;
        glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5f);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5f);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5f);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5f);
    }
    glEnd();

    glNormal3f(0.0f, 0.0f, -1.0f);
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.0 * PI / teeth;
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5f);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5f);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5f);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5f);
    }
    glEnd();

    glBegin(GL_QUADS);
    for (i = 0; i < teeth; i++) {
        angle = i * 2.0 * PI / teeth;
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5f);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5f);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5f);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5f);
    }
    glEnd();

    glBegin(GL_QUAD_STRIP);
    for (i = 0; i < teeth; i++) {
        angle = i * 2.0 * PI / teeth;
        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5f);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5f);
        GLfloat u = r2 * cos(angle + da) - r1 * cos(angle);
        GLfloat v = r2 * sin(angle + da) - r1 * sin(angle);
        GLfloat len = sqrt(u * u + v * v);
        glNormal3f(v / len, -u / len, 0.0f);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5f);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5f);
        glNormal3f(cos(angle), sin(angle), 0.0f);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5f);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5f);
        u = r1 * cos(angle + 3 * da) - r2 * cos(angle + 2 * da);
        v = r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da);
        glNormal3f(v / len, -u / len, 0.0f);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5f);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5f);
        glNormal3f(cos(angle), sin(angle), 0.0f);
    }
    glVertex3f(r1 * cos(0.0f), r1 * sin(0.0f), width * 0.5f);
    glVertex3f(r1 * cos(0.0f), r1 * sin(0.0f), -width * 0.5f);
    glEnd();

    glShadeModel(GL_SMOOTH);
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.0 * PI / teeth;
        glNormal3f(-cos(angle), -sin(angle), 0.0f);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5f);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5f);
    }
    glEnd();
}

/******************************************
 * Draw an approximated chain
 ******************************************/
void drawChain()
{
    GLfloat depth;
    static int mode = 0;

    glColor3f(0.0f, 1.0f, 0.5f);
    glEnable(GL_LINE_STIPPLE);
    mode = (mode + 1) % 2;

    if (mode == 0 && Abs(speed) > 0) glLineStipple(1, 0x1c47);
    else if (mode == 1 && Abs(speed) > 0) glLineStipple(1, 0x00FF);

    glBegin(GL_LINES);
    for (depth = 0.06f; depth <= 0.12f; depth += 0.01f) {
        glVertex3f(-1.6f, 0.15f, ROD_RADIUS);
        glVertex3f(0.0f, 0.3f, depth);
        glVertex3f(-1.6f, -0.15f, ROD_RADIUS);
        glVertex3f(0.0f, -0.3f, depth);
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);
}

/******************************************
 * Draw the seat
 ******************************************/
void drawSeat()
{
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_POLYGON);
        glVertex3f(-0.20f, 1.2f, -0.60f);
        glVertex3f(1.2f, 1.0f, -0.40f);
        glVertex3f(1.0f, 1.1f, 0.30f);
        glVertex3f(-0.20f, 1.3f, 0.70f);
        glVertex3f(-0.70f, 1.0f, 1.2f);
        glVertex3f(-1.2f, 1.1f, 1.2f);
        glVertex3f(-1.2f, 1.0f, -1.2f);
        glVertex3f(-0.70f, 1.0f, -1.2f);
    glEnd();

    glColor3f(0.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
        glVertex3f(-0.20f, -1.2f, -0.60f);
        glVertex3f(1.2f, -1.0f, -0.40f);
        glVertex3f(1.0f, -1.1f, 0.30f);
        glVertex3f(-0.20f, -1.3f, 0.70f);
        glVertex3f(-0.70f, -1.0f, 1.2f);
        glVertex3f(-1.2f, -1.1f, 1.2f);
        glVertex3f(-1.2f, -1.0f, -1.2f);
        glVertex3f(-0.70f, -1.0f, -1.2f);
    glEnd();

    glBegin(GL_QUADS);
        glVertex3f(1.2f, 1.0f, -0.40f); glVertex3f(1.2f, 1.0f, 0.30f);
        glVertex3f(1.2f, -1.0f, 0.30f); glVertex3f(1.2f, -1.0f, -0.40f);
        glVertex3f(1.2f, 1.0f, 0.30f); glVertex3f(-0.20f, 1.3f, 0.70f);
        glVertex3f(-0.20f, -1.3f, 0.70f); glVertex3f(1.2f, -1.0f, 0.30f);
        glVertex3f(1.2f, 1.0f, -0.40f); glVertex3f(-0.20f, 1.2f, -0.60f);
        glVertex3f(-0.20f, -1.2f, -0.60f); glVertex3f(1.2f, -1.0f, -0.40f);
        glVertex3f(-0.20f, 1.3f, 0.70f); glVertex3f(-0.70f, 1.0f, 1.2f);
        glVertex3f(-0.70f, -1.0f, 1.2f); glVertex3f(-0.20f, -1.3f, 0.70f);
        glVertex3f(-0.20f, 1.2f, -0.60f); glVertex3f(-0.70f, 1.0f, -1.2f);
        glVertex3f(-0.70f, -1.0f, -1.2f); glVertex3f(-0.20f, -1.2f, -0.60f);
        glVertex3f(-0.70f, 1.0f, 1.2f); glVertex3f(-1.2f, 1.0f, 1.2f);
        glVertex3f(-1.2f, -1.0f, 1.2f); glVertex3f(-0.70f, -1.0f, 1.2f);
        glVertex3f(-0.70f, 1.0f, -1.2f); glVertex3f(-1.2f, 1.0f, -1.2f);
        glVertex3f(-1.2f, -1.0f, -1.2f); glVertex3f(-1.2f, -1.0f, 1.2f);
    glEnd();
}

/******************************************
 * Draw the pedals
 ******************************************/
void drawPedals()
{
    glColor3f(0.25f, 0.15f, 0.1f);
    glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.105f);
        glRotatef(-pedalAngle, 0.0f, 0.0f, 1.0f);
        glTranslatef(0.25f, 0.0f, 0.0f);
        glPushMatrix();
            glScalef(0.5f, 0.1f, 0.1f);
            glutSolidCube(1.0f);
        glPopMatrix();
        glPushMatrix();
            glTranslatef(0.25f, 0.0f, 0.15f);
            glRotatef(pedalAngle, 0.0f, 0.0f, 1.0f);
            glScalef(0.2f, 0.02f, 0.3f);
            glutSolidCube(1.0f);
        glPopMatrix();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.0f, 0.0f, -0.105f);
        glRotatef(180.0f - pedalAngle, 0.0f, 0.0f, 1.0f);
        glTranslatef(0.25f, 0.0f, 0.0f);
        glPushMatrix();
            glScalef(0.5f, 0.1f, 0.1f);
            glutSolidCube(1.0f);
        glPopMatrix();
        glPushMatrix();
            glTranslatef(0.25f, 0.0f, -0.15f);
            glRotatef(pedalAngle - 180.0f, 0.0f, 0.0f, 1.0f);
            glScalef(0.2f, 0.02f, 0.3f);
            glutSolidCube(1.0f);
        glPopMatrix();
    glPopMatrix();

    glColor3f(0.4f, 0.0f, 0.0f);
}

/******************************************
 * Draw a tyre
 ******************************************/
void drawTyre(void)
{
    int i;
    glColor3f(0.3f, 0.0f, 0.3f);
    glutSolidTorus(0.06f, 0.92f, 4, 30);
    glColor3f(1.0f, 1.0f, 0.5f);
    glPushMatrix();
        glTranslatef(0.0f, 0.0f, -0.06f);
        ZCylinder(0.02f, 0.12f);
    glPopMatrix();
    glutSolidTorus(0.02f, 0.02f, 3, 20);
    glColor3f(0.8f, 0.6f, 0.5f);
    for (i = 0; i < NUM_SPOKES; ++i) {
        glPushMatrix();
            glRotatef(i * SPOKE_ANGLE, 0.0f, 0.0f, 1.0f);
            glBegin(GL_LINES);
                glVertex3f(0.0f, 0.02f, 0.0f);
                glVertex3f(0.0f, 0.86f, 0.0f);
            glEnd();
        glPopMatrix();
    }
    glColor3f(0.0f, 0.0f, 0.0f);
    glutSolidTorus(TUBE_WIDTH, RADIUS_WHEEL, 10, 30);
    glColor3f(0.4f, 0.0f, 0.0f);
}

/******************************************
 * Draw the person on the bicycle
 ******************************************/
void drawPerson(void)
{
    glColor3f(0.8f, 0.6f, 0.4f); // Skin-like color for body parts

    // Position the person at the seat (adjust based on seat transformations)
    glPushMatrix();
        // Translate to seat position (approximated from drawFrame)
        glTranslatef(-0.2f, 0.3f, 0.0f); // Slight offset above seat
        // Adjust person's lean based on wheelie
        glRotatef(-10.0f + wheelieAngle * 0.5f, 0.0f, 0.0f, 1.0f); // Lean forward slightly, adjust with wheelie

        // Torso (cylinder along Y-axis)
        glPushMatrix();
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Align along Y
            ZCylinder(0.15f, 0.6f); // Torso: 0.6 units long
        glPopMatrix();

        // Head (sphere above torso)
        glPushMatrix();
            glTranslatef(0.0f, 0.7f, 0.0f);
            glutSolidSphere(0.1f, 10, 10);
        glPopMatrix();

        // Left Arm (upper and lower arm with elbow bend)
        glPushMatrix();
            glTranslatef(0.2f, 0.5f, 0.0f); // Shoulder position
            glRotatef(-45.0f + wheelieAngle * 0.3f, 0.0f, 0.0f, 1.0f); // Angle toward handlebar, adjust with wheelie
            // Upper arm
            glPushMatrix();
                ZCylinder(0.05f, 0.3f);
            glPopMatrix();
            // Lower arm
            glPushMatrix();
                glTranslatef(0.0f, 0.0f, 0.3f);
                glRotatef(-30.0f, 0.0f, 0.0f, 1.0f); // Elbow bend
                ZCylinder(0.05f, 0.3f);
            glPopMatrix();
        glPopMatrix();

        // Right Arm (symmetric to left)
        glPushMatrix();
            glTranslatef(-0.2f, 0.5f, 0.0f);
            glRotatef(-45.0f + wheelieAngle * 0.3f, 0.0f, 0.0f, 1.0f);
            // Upper arm
            glPushMatrix();
                ZCylinder(0.05f, 0.3f);
            glPopMatrix();
            // Lower arm
            glPushMatrix();
                glTranslatef(0.0f, 0.0f, 0.3f);
                glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);
                ZCylinder(0.05f, 0.3f);
            glPopMatrix();
        glPopMatrix();

        // Left Leg (upper and lower leg synchronized with pedal)
        glPushMatrix();
            glTranslatef(0.1f, 0.0f, 0.105f); // Hip position, aligned with pedal
            glRotatef(-pedalAngle - 90.0f, 0.0f, 0.0f, 1.0f); // Sync with pedal
            // Upper leg (thigh)
            glPushMatrix();
                ZCylinder(0.07f, 0.4f);
            glPopMatrix();
            // Lower leg (calf)
            glPushMatrix();
                glTranslatef(0.0f, 0.0f, 0.4f);
                glRotatef(60.0f * sin(radians(pedalAngle)) + 30.0f, 0.0f, 0.0f, 1.0f); // Knee bend
                ZCylinder(0.07f, 0.4f);
            glPopMatrix();
        glPopMatrix();

        // Right Leg (opposite phase)
        glPushMatrix();
            glTranslatef(-0.1f, 0.0f, -0.105f);
            glRotatef(180.0f - pedalAngle - 90.0f, 0.0f, 0.0f, 1.0f);
            // Upper leg
            glPushMatrix();
                ZCylinder(0.07f, 0.4f);
            glPopMatrix();
            // Lower leg
            glPushMatrix();
                glTranslatef(0.0f, 0.0f, 0.4f);
                glRotatef(60.0f * sin(radians(pedalAngle + 180.0f)) + 30.0f, 0.0f, 0.0f, 1.0f);
                ZCylinder(0.07f, 0.4f);
            glPopMatrix();
        glPopMatrix();

    glPopMatrix();

    glColor3f(0.4f, 0.0f, 0.0f); // Restore frame color
}

/******************************************
 * Draw on-screen control labels
 ******************************************/
void drawControlsText(void)
{
    // Switch to 2D orthographic projection for text rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WIN_WIDTH, 0, WIN_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Disable lighting and depth test for 2D text
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Set text color to yellow
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow color

    // Define starting position for text (top-left corner with padding)
    int x = 10;
    int y = WIN_HEIGHT - 20;

    // Array of control strings
    const char *controls[] = {
        "Project of: NGUYEN HUU PHONG & HOANG MINH TRI",
        "R: return",
        "W: Move forward",
        "A: Steer left",
        "S: Move backward",
        "D: Steer right",

    };
    int numControls = sizeof(controls) / sizeof(controls[0]);
    void *font = GLUT_BITMAP_HELVETICA_12;

    // Buffer to store the speed string
    char speedStr[32];
    sprintf(speedStr, "Speed: %.2f", speed);

    // Draw each control label
    for (int i = 0; i < numControls; i++) {
        glRasterPos2i(x, y - i * 15); // Move down for each line (15 pixels spacing)
        const char *str = controls[i];
        for (int j = 0; j < strlen(str); j++) {
            glutBitmapCharacter(font, str[j]);
        }
    }

    // Draw the speed label below the controls
    glRasterPos2i(x, y - numControls * 15);
    for (int j = 0; j < strlen(speedStr); j++) {
        glutBitmapCharacter(font, speedStr[j]);
    }

    // Restore 3D rendering settings
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/******************************************
 * Initialize OpenGL settings
 ******************************************/
void init()
{
    GLfloat mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat mat_shininess[] = {100.0f};
    GLfloat light_directional[] = {1.0f, 1.0f, 1.0f, 0.0f};
    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};

    reset();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glShadeModel(GL_SMOOTH);
    glLightfv(GL_LIGHT0, GL_POSITION, light_directional);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glColorMaterial(GL_FRONT, GL_DIFFUSE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
}

/******************************************
 * Draw ground grid
 ******************************************/
void landmarks(void)
{
    GLfloat i;
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    for (i = -100.0f; i <= 100.0f; i += 1.0f) {
        glVertex3f(-100.0f, -RADIUS_WHEEL, i);
        glVertex3f(100.0f, -RADIUS_WHEEL, i);
        glVertex3f(i, -RADIUS_WHEEL, -100.0f);
        glVertex3f(i, -RADIUS_WHEEL, 100.0f);
    }
    glEnd();
}

/******************************************
 * Display function
 ******************************************/
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(camx, camy, camz, xpos, 0.0f, zpos, 0.0f, 1.0f, 0.0f);

    glPushMatrix();
        glRotatef(angley, 1.0f, 0.0f, 0.0f);
        glRotatef(anglex, 0.0f, 1.0f, 0.0f);
        glRotatef(anglez, 0.0f, 0.0f, 1.0f);

        landmarks();

        glPushMatrix();
            glTranslatef(xpos, 0.0f, zpos);
            glRotatef(direction, 0.0f, 1.0f, 0.0f);
            drawFrame();
            drawChain();
            drawPedals();
            drawPerson();
        glPopMatrix();
    glPopMatrix();

    // Draw the on-screen control labels
    drawControlsText();

    glutSwapBuffers();
}

/******************************************
 * Utility functions
 ******************************************/
GLfloat Abs(GLfloat a)
{
    return (a < 0.0f) ? -a : a;
}

GLfloat degrees(GLfloat a)
{
    return a * 180.0f / PI;
}

GLfloat radians(GLfloat a)
{
    return a * PI / 180.0f;
}

/******************************************
 * Idle function
 ******************************************/
void idle(void)
{
    updateScene();
    glutPostRedisplay();
}

/******************************************
 * Timer function to reset wheelie
 ******************************************/
void wheelieReset(int value)
{
    if (wheelieActive) {
        wheelieAngle = 0.0f;
        wheelieActive = 0;
        wheelieTimer = 0;
    }
}

/******************************************
 * Special key handler
 ******************************************/
void special(int key, int x, int y)
{
    switch (key) {
        case GLUT_KEY_UP: camz -= 0.1f; break;
        case GLUT_KEY_DOWN: camz += 0.1f; break;
        case GLUT_KEY_LEFT: camx -= 0.1f; break;
        case GLUT_KEY_RIGHT: camx += 0.1f; break;
    }
    glutPostRedisplay();
}

/******************************************
 * Reset scene
 ******************************************/
void reset()
{
    anglex = angley = anglez = 0.0f;
    pedalAngle = steering = 0.0f;
    Mouse = GLUT_UP;
    speed = 0.0f;
    camx = 0.0f;
    camy = 2.0f;
    camz = 5.0f;
    xpos = zpos = 0.0f;
    direction = 0.0f;
    wheelieAngle = 0.0f;
    wheelieActive = 0;
    wheelieTimer = 0;
}

/******************************************
 * Keyboard handler
 ******************************************/
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 'w':
        case 'W':
            // Set direction to forward (positive speed)
            if (speed >= 0.0f) {
                speed += INC_SPEED; // Increase speed if already moving forward
            } else {
                speed = -speed; // Reverse direction if moving backward
                speed += INC_SPEED; // Then increase speed
            }
            if (speed > MAX_SPEED) speed = MAX_SPEED;
            break;
        case 's':
        case 'S':
            // Set direction to backward (negative speed)
            if (speed <= 0.0f) {
                speed -= INC_SPEED; // Decrease speed if already moving backward
            } else {
                speed = -speed; // Reverse direction if moving forward
                speed -= INC_SPEED; // Then decrease speed
            }
            if (speed < MIN_SPEED) speed = MIN_SPEED;
            break;
        case 'a':
        case 'A':
            if (steering < HANDLE_LIMIT) steering += INC_STEERING;
            break;
        case 'd':
        case 'D':
            if (steering > -HANDLE_LIMIT) steering -= INC_STEERING;
            break;
        case '+':
            // Increase the absolute speed (go faster) while keeping direction
            if (speed >= 0.0f) {
                speed += INC_SPEED;
                if (speed > MAX_SPEED) speed = MAX_SPEED;
            } else {
                speed -= INC_SPEED; // Speed is negative, so subtract to increase absolute value
                if (speed < MIN_SPEED) speed = MIN_SPEED;
            }
            break;
        case '-':
            // Decrease the absolute speed (go slower) while keeping direction
            if (speed >= 0.0f) {
                speed -= INC_SPEED;
                if (speed < 0.0f) speed = 0.0f; // Don't let it go negative
            } else {
                speed += INC_SPEED; // Speed is negative, so add to decrease absolute value
                if (speed > 0.0f) speed = 0.0f; // Don't let it go positive
            }
            break;
        case 'q':
        case 'Q':
            if (!wheelieActive) {
                wheelieAngle = WHEELIE_ANGLE; // Set wheelie angle
                wheelieActive = 1;
                wheelieTimer = glutGet(GLUT_ELAPSED_TIME);
                glutTimerFunc(WHEELIE_DURATION, wheelieReset, 0); // Reset after duration
            }
            break;
        case 'r':
        case 'R':
            reset();
            break;
        case 27:
            exit(0);
            break;
    }
    glutPostRedisplay();
}

/******************************************
 * Mouse handler
 ******************************************/
void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            Mouse = GLUT_DOWN;
            prevx = x;
            prevy = y;
        } else {
            Mouse = GLUT_UP;
        }
    }
    glutPostRedisplay();
}

/******************************************
 * Motion handler
 ******************************************/
void motion(int x, int y)
{
    if (Mouse == GLUT_DOWN) {
        int deltax = prevx - x;
        int deltay = prevy - y;
        anglex += 0.5f * deltax;
        angley += 0.5f * deltay;
        if (deltax != 0 && deltay != 0)
            anglez += 0.5f * sqrt((float)(deltax * deltax + deltay * deltay));

        while (anglex >= 360.0f) anglex -= 360.0f;
        while (angley >= 360.0f) angley -= 360.0f;
        while (anglez >= 360.0f) anglez -= 360.0f;
        while (anglex < 0.0f) anglex += 360.0f;
        while (angley < 0.0f) angley += 360.0f;
        while (anglez < 0.0f) anglez += 360.0f;
    }
    prevx = x;
    prevy = y;
    glutPostRedisplay();
}

/******************************************
 * Passive motion handler
 ******************************************/
void passive(int x, int y)
{
}

/******************************************
 * Reshape handler
 ******************************************/
void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

/******************************************
 * Setup GLUT callbacks
 ******************************************/
void glSetupFuncs(void)
{
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(passive);
    glutSetCursor(GLUT_CURSOR_CROSSHAIR);
}

/******************************************
 * Help text
 ******************************************/
void help(void)
{
    printf("Hierarchical 3D Model of a Bicycle with a Person\n");
    printf("Controls:\n");
    printf("  W: Move forward\n");
    printf("  S: Move backward\n");
    printf("  A: Steer left\n");
    printf("  D: Steer right\n");
    printf("  +: Increase speed\n");
    printf("  -: Decrease speed\n");
    printf("  Q: Perform wheelie\n");
    printf("  R: Reset scene\n");
    printf("  Arrow keys: Move camera\n");
    printf("  Mouse drag: Rotate scene\n");
    printf("  ESC: Exit\n");
}

/******************************************
 * Main function
 ******************************************/
int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
    glutCreateWindow("Bicycle with Person - 3D Model with Controls");
    init();
    glSetupFuncs();
    help();
    glutMainLoop();
    return 0;
}
