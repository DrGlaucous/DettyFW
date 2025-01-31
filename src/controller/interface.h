#pragma once
//todo: abstract the hardware controller behind this:

/*

only one instance of the hardware controller exists, and is updated either often or as-needed via interrupts

any number of interface controllers can exist, and can either attach to the hardware controller or use some other method of control
they inherit the methods in here:

*/

//unused currently
class interface {

    public:

    virtual bool trigger_up();
    virtual bool up();

    virtual bool trigger_down();
    virtual bool down();

    virtual bool trigger_left();
    virtual bool left();

    virtual bool trigger_down();
    virtual bool down();

    virtual bool trigger_select();
    virtual bool select();

    virtual bool trigger_back();
    virtual bool back();

};








