#ifndef __netlist_H
#define __netlist_H
/*
 * Copyright (c) 1998-2002 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: netlist.h,v 1.231 2002/01/28 00:52:41 steve Exp $"
#endif

/*
 * The netlist types, as described in this header file, are intended
 * to be the output from elaboration of the source design. The design
 * can be passed around in this form to the various stages and design
 * processors.
 */
# include  <string>
# include  <map>
# include  <list>
# include  "verinum.h"
# include  "HName.h"
# include  "LineInfo.h"
# include  "svector.h"
# include  "Attrib.h"

#ifdef HAVE_IOSFWD
# include  <iosfwd>
#else
class ostream;
#endif

class Design;
class Link;
class Nexus;
class NetNode;
class NetProc;
class NetProcTop;
class NetScope;
class NetExpr;
class NetESignal;


struct target;
struct functor_t;

/* =========
 * A NetObj is anything that has any kind of behavior in the
 * netlist. Nodes can be gates, registers, etc. and are linked
 * together to form a design web.
 *
 * The web of nodes that makes up a circuit is held together by the
 * Link class. There is a link for each pin. All mutually connected
 * pins form a ring of links.
 *
 * A link can be INPUT, OUTPUT or PASSIVE. An input never drives the
 * signal, and PASSIVE never receives the value of the signal. Wires
 * are PASSIVE, for example.
 *
 * A NetObj also has delays specified as rise_time, fall_time and
 * decay_time. The rise and fall time are the times to transition to 1
 * or 0 values. The decay_time is the time needed to decay to a 'bz
 * value, or to decay of the net is a trireg. The exact and precise
 * interpretation of the rise/fall/decay times is typically left to
 * the target to properly interpret.
 */
class NetObj {

    public:
    public:
      explicit NetObj(NetScope*s, const string&n, unsigned npins);
      explicit NetObj(NetScope*s, const char*n, unsigned npins);
      virtual ~NetObj();

      NetScope* scope();
      const NetScope* scope() const;

      const char* name() const { return name_; }

      unsigned pin_count() const { return npins_; }

      unsigned rise_time() const { return delay1_; }
      unsigned fall_time() const { return delay2_; }
      unsigned decay_time() const { return delay3_; }

      void rise_time(unsigned d) { delay1_ = d; }
      void fall_time(unsigned d) { delay2_ = d; }
      void decay_time(unsigned d) { delay3_ = d; }

      void set_attributes(const map<string,string>&);
      string attribute(const string&key) const;
      void attribute(const string&key, const string&value);

	// Return true if this has all the attributes in that and they
	// all have the same values.
      bool has_compat_attributes(const NetObj&that) const;

      unsigned nattr() const;
      const char* attr_key(unsigned) const;
      const char* attr_value(unsigned) const;

      Link&pin(unsigned idx);
      const Link&pin(unsigned idx) const;

      void dump_node_pins(ostream&, unsigned) const;
      void dump_obj_attr(ostream&, unsigned) const;

    private:
      NetScope*scope_;
      char* name_;
      Link*pins_;
      const unsigned npins_;
      unsigned delay1_;
      unsigned delay2_;
      unsigned delay3_;

      Attrib attributes_;
};

class Link {

      friend void connect(Link&, Link&);
      friend class NetObj;
      friend class Nexus;

    public:
      enum DIR { PASSIVE, INPUT, OUTPUT };

      enum strength_t { HIGHZ, WEAK, PULL, STRONG, SUPPLY };

      Link();
      ~Link();

	// Manipulate the link direction.
      void set_dir(DIR d);
      DIR get_dir() const;

	// A link has a drive strength for 0 and 1 values. The drive0
	// strength is for when the link has the value 0, and drive1
	// strength is for when the link has a value 1.
      void drive0(strength_t);
      void drive1(strength_t);

      strength_t drive0() const;
      strength_t drive1() const;

	// A link has an initial value that is used by the nexus to
	// figure out its initial value. Normally, only the object
	// that contains the link sets the initial value, and only the
	// attached Nexus gets it. The default link value is Vx.
      void set_init(verinum::V val);
      verinum::V get_init() const;

      void cur_link(NetObj*&net, unsigned &pin);
      void cur_link(const NetObj*&net, unsigned &pin) const;

	// Get a pointer to the nexus that represents all the links
	// connected to me.
      Nexus* nexus();
      const Nexus* nexus()const;

	// Return a pointer to the next link in the nexus.
      Link* next_nlink();
      const Link* next_nlink() const;

	// Remove this link from the set of connected pins. The
	// destructor will automatically do this if needed.
      void unlink();

	// Return true if this link is connected to anything else.
      bool is_linked() const;

	// Return true if these pins are connected.
      bool is_linked(const Link&that) const;

	// Return true if this is the same pin of the same object of
	// that link.
      bool is_equal(const Link&that) const;

	// Return information about the object that this link is
	// a part of.
      const NetObj*get_obj() const;
      NetObj*get_obj();
      unsigned get_pin() const;

	// A link of an object (sometimes called a "pin") has a
	// name. It is convenient for the name to have a string and an
	// integer part.
      void set_name(const string&, unsigned inst =0);
      const string& get_name() const;
      unsigned get_inst() const;

    private:
	// The NetNode manages these. They point back to the
	// NetNode so that following the links can get me here.
      NetObj *node_;
      unsigned pin_;

      DIR dir_;
      strength_t drive0_, drive1_;
      verinum::V init_;

	// These members name the pin of the link. If the name
	// has width, then the ninst_ member is the index of the
	// pin.
      string   name_;
      unsigned inst_;

    private:
      Link *next_;
      Nexus*nexus_;

    private: // not implemented
      Link(const Link&);
      Link& operator= (const Link&);
};


/*
 * The Nexus represents a collection of links that are joined
 * together. Each link has its own properties, this class holds the
 * properties of the group.
 *
 * The links in a nexus are grouped into a singly linked list, with
 * the nexus pointing to the first Link. Each link in turn points to
 * the next link in the nexus, with the last link pointing to 0.
 *
 * The t_cookie() is a void* that targets can use to store information
 * in a Nexus. ivl guarantees that the t_cookie will be 0 when the
 * target is invoked.
 */
class Nexus {

      friend void connect(Link&, Link&);
      friend class Link;

    public:
      explicit Nexus();
      ~Nexus();

      const char* name() const;
      verinum::V get_init() const;

      Link*first_nlink();
      const Link* first_nlink()const;

      void* t_cookie() const;
      void* t_cookie(void*) const;

    private:
      Link*list_;
      void unlink(Link*);
      void relink(Link*);

      mutable char* name_; /* Cache the calculated name for the Nexus. */
      mutable void* t_cookie_;

    private: // not implemented
      Nexus(const Nexus&);
      Nexus& operator= (const Nexus&);
};


/*
 * A NetNode is a device of some sort, where each pin has a different
 * meaning. (i.e. pin(0) is the output to an and gate.) NetNode
 * objects are listed in the nodes_ of the Design object.
 */
class NetNode  : public NetObj {

    public:
      explicit NetNode(NetScope*s, const string&n, unsigned npins);
      explicit NetNode(NetScope*s, const char*n, unsigned npins);

      virtual ~NetNode();

	// This method locates the next node that has all its pins
	// connected to the same of my own pins.
      NetNode*next_node();

      virtual bool emit_node(struct target_t*) const;
      virtual void dump_node(ostream&, unsigned) const;

	// This is used to scan a modifiable netlist, one node at a time.
      virtual void functor_node(Design*, functor_t*);

    private:
      friend class Design;
      NetNode*node_next_, *node_prev_;
      Design*design_;
};


/*
 * NetNet is a special kind of NetObj that doesn't really do anything,
 * but carries the properties of the wire/reg/trireg, including its
 * name. A scaler wire is a NetNet with one pin, a vector a wider
 * NetNet. NetNet objects also appear as side effects of synthesis or
 * other abstractions.
 *
 * Note that there are no INTEGER types. Express a verilog integer as
 * a ``reg signed'' instead. The parser automatically does this for us.
 *
 * NetNet objects have a name and exist within a scope, so the
 * constructor takes a pointer to the containing scope. The object
 * automatically adds itself to the scope.
 *
 * NetNet objects are located by searching NetScope objects.
 *
 * All the pins of a NetNet object are PASSIVE: they do not drive
 * anything and they are not a data sink, per se. The pins follow the
 * values on the nexus.
 */
class NetNet  : public NetObj, public LineInfo {

    public:
      enum Type { IMPLICIT, IMPLICIT_REG, WIRE, TRI, TRI1, SUPPLY0,
		  SUPPLY1, WAND, TRIAND, TRI0, WOR, TRIOR, REG };

      enum PortType { NOT_A_PORT, PIMPLICIT, PINPUT, POUTPUT, PINOUT };

      explicit NetNet(NetScope*s, const string&n, Type t, unsigned npins =1);

      explicit NetNet(NetScope*s, const string&n, Type t, long ms, long ls);

      virtual ~NetNet();

      Type type() const;

      PortType port_type() const;
      void port_type(PortType t);

	/* If a NetNet is signed, then its value is to be treated as
	   signed. Otherwise, it is unsigned. */
      bool get_signed() const;
      void set_signed(bool);

	/* These methods return the msb and lsb indices for the most
	   significant and least significant bits. These are signed
	   longs, and may be different from pin numbers. For example,
	   reg [1:8] has 8 bits, msb==1 and lsb==8. */
      long msb() const;
      long lsb() const;

	/* This method converts a signed index (the type that might be
	   found in the verilog source) to a pin number. It accounts
	   for variation in the definition of the reg/wire/whatever. */
      unsigned sb_to_idx(long sb) const;

      bool local_flag() const { return local_flag_; }
      void local_flag(bool f) { local_flag_ = f; }

	/* NetESignal objects may reference this object. Keep a
	   reference count so that I keep track of them. */
      void incr_eref();
      void decr_eref();
      unsigned get_eref() const;


      virtual void dump_net(ostream&, unsigned) const;

    private:
	// The NetScope class uses this for listing signals.
      friend class NetScope;
      NetNet*sig_next_, *sig_prev_;

    private:
      Type   type_;
      PortType port_type_;
      bool signed_;

      long msb_, lsb_;

      bool local_flag_;
      unsigned eref_count_;
};

/*
 * This class implements the LPM_ADD_SUB component as described in the
 * EDIF LPM Version 2 1 0 standard. It is used as a structural
 * implementation of the + and - operators.
 */
class NetAddSub  : public NetNode {

    public:
      NetAddSub(NetScope*s, const string&n, unsigned width);
      ~NetAddSub();

	// Get the width of the device (that is, the width of the
	// operands and results.)
      unsigned width() const;

      Link& pin_Aclr();
      Link& pin_Add_Sub();
      Link& pin_Clock();
      Link& pin_Cin();
      Link& pin_Cout();
      Link& pin_Overflow();

      Link& pin_DataA(unsigned idx);
      Link& pin_DataB(unsigned idx);
      Link& pin_Result(unsigned idx);

      const Link& pin_Cout() const;
      const Link& pin_DataA(unsigned idx) const;
      const Link& pin_DataB(unsigned idx) const;
      const Link& pin_Result(unsigned idx) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);
};

/*
 * This type represents the LPM_CLSHIFT device.
 */
class NetCLShift  : public NetNode {

    public:
      NetCLShift(NetScope*s, const string&n, unsigned width,
		 unsigned width_dist);
      ~NetCLShift();

      unsigned width() const;
      unsigned width_dist() const;

      Link& pin_Direction();
      Link& pin_Underflow();
      Link& pin_Overflow();
      Link& pin_Data(unsigned idx);
      Link& pin_Result(unsigned idx);
      Link& pin_Distance(unsigned idx);

      const Link& pin_Direction() const;
      const Link& pin_Underflow() const;
      const Link& pin_Overflow() const;
      const Link& pin_Data(unsigned idx) const;
      const Link& pin_Result(unsigned idx) const;
      const Link& pin_Distance(unsigned idx) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      unsigned width_;
      unsigned width_dist_;
};

/*
 * This class supports the LPM_COMPARE device.
 *
 * The width of the device is the width of the inputs. If one of the
 * inputs is narrower then the other, it is up to the generator to
 * make sure all the data pins are properly driven.
 *
 * NOTE: This is not the same as the device used to support case
 * compare. Case comparisons handle Vx and Vz values, whereas this
 * device need not.
 */
class NetCompare  : public NetNode {

    public:
      NetCompare(NetScope*scope, const string&n, unsigned width);
      ~NetCompare();

      unsigned width() const;

      Link& pin_Aclr();
      Link& pin_Clock();
      Link& pin_AGB();
      Link& pin_AGEB();
      Link& pin_AEB();
      Link& pin_ANEB();
      Link& pin_ALB();
      Link& pin_ALEB();

      Link& pin_DataA(unsigned idx);
      Link& pin_DataB(unsigned idx);

      const Link& pin_Aclr() const;
      const Link& pin_Clock() const;
      const Link& pin_AGB() const;
      const Link& pin_AGEB() const;
      const Link& pin_AEB() const;
      const Link& pin_ANEB() const;
      const Link& pin_ALB() const;
      const Link& pin_ALEB() const;

      const Link& pin_DataA(unsigned idx) const;
      const Link& pin_DataB(unsigned idx) const;

      virtual void functor_node(Design*, functor_t*);
      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      unsigned width_;
};

/*
 * This class represents a theoretical (though not necessarily
 * practical) integer divider gate. This is not to represent any real
 * hardware, but to support the / operator in Verilog, when it shows
 * up in structural contexts.
 *
 * The operands of the operation are the DataA<i> and DataB<i> inputs,
 * and the Result<i> output reflects the value DataA/DataB.
 */

class NetDivide  : public NetNode {

    public:
      NetDivide(NetScope*scope, const string&n,
		unsigned width, unsigned wa, unsigned wb);
      ~NetDivide();

      unsigned width_r() const;
      unsigned width_a() const;
      unsigned width_b() const;

      Link& pin_DataA(unsigned idx);
      Link& pin_DataB(unsigned idx);
      Link& pin_Result(unsigned idx);

      const Link& pin_DataA(unsigned idx) const;
      const Link& pin_DataB(unsigned idx) const;
      const Link& pin_Result(unsigned idx) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned width_r_;
      unsigned width_a_;
      unsigned width_b_;
};

/*
 * This class represents a theoretical (though not necessarily
 * practical) integer modulo gate. This is not to represent any real
 * hardware, but to support the % operator in Verilog, when it shows
 * up in structural contexts.
 *
 * The operands of the operation are the DataA<i> and DataB<i> inputs,
 * and the Result<i> output reflects the value DataA%DataB.
 */

class NetModulo  : public NetNode {

    public:
      NetModulo(NetScope*s, const string&n,
		unsigned width, unsigned wa, unsigned wb);
      ~NetModulo();

      unsigned width_r() const;
      unsigned width_a() const;
      unsigned width_b() const;

      Link& pin_DataA(unsigned idx);
      Link& pin_DataB(unsigned idx);
      Link& pin_Result(unsigned idx);

      const Link& pin_DataA(unsigned idx) const;
      const Link& pin_DataB(unsigned idx) const;
      const Link& pin_Result(unsigned idx) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned width_r_;
      unsigned width_a_;
      unsigned width_b_;
};

/*
 * This class represents an LPM_FF device. There is no literal gate
 * type in Verilog that maps, but gates of this type can be inferred.
 */
class NetFF  : public NetNode {

    public:
      NetFF(NetScope*s, const char*n, unsigned width);
      ~NetFF();

      unsigned width() const;

      Link& pin_Clock();
      Link& pin_Enable();
      Link& pin_Aload();
      Link& pin_Aset();
      Link& pin_Aclr();
      Link& pin_Sload();
      Link& pin_Sset();
      Link& pin_Sclr();

      Link& pin_Data(unsigned);
      Link& pin_Q(unsigned);

      const Link& pin_Clock() const;
      const Link& pin_Enable() const;
      const Link& pin_Data(unsigned) const;
      const Link& pin_Q(unsigned) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);
};


/*
 * This class represents the declared memory object. The parser
 * creates one of these for each declared memory in the elaborated
 * design. A reference to one of these is handled by the NetEMemory
 * object, which is derived from NetExpr. This is not a node because
 * memory objects can only be accessed by behavioral code.
 */
class NetMemory  {

    public:
      NetMemory(NetScope*sc, const string&n, long w, long s, long e);
      ~NetMemory();

      const string&name() const { return name_; }

	// This is the width (in bits) of a single memory position.
      unsigned width() const { return width_; }

      // NetScope*scope();
      const NetScope*scope() const { return scope_; };

	// This is the number of memory positions.
      unsigned count() const;

	// This method returns a 0 based address of a memory entry as
	// indexed by idx. The Verilog source may give index ranges
	// that are not zero based.
      unsigned index_to_address(long idx) const;

      void dump(ostream&o, unsigned lm) const;

    private:
      string name_;
      unsigned width_;
      long idxh_;
      long idxl_;

      friend class NetRamDq;
      NetRamDq* ram_list_;

      friend class NetScope;
      NetMemory*snext_, *sprev_;
      NetScope*scope_;

    private: // not implemented
      NetMemory(const NetMemory&);
      NetMemory& operator= (const NetMemory&);
};

/*
 * This class implements the LPM_MULT component as described in the
 * EDIF LPM Version 2 1 0 standard. It is used as a structural
 * implementation of the * operator. The device has inputs DataA and
 * DataB that can have independent widths, as can the result. If the
 * result is smaller then the widths of a and b together, then the
 * device drops the least significant bits of the product.
 */
class NetMult  : public NetNode {

    public:
      NetMult(NetScope*sc, const string&n, unsigned width,
	      unsigned wa, unsigned wb, unsigned width_s =0);
      ~NetMult();

	// Get the width of the device bussed inputs. There are these
	// parameterized widths:
      unsigned width_r() const; // Result
      unsigned width_a() const; // DataA
      unsigned width_b() const; // DataB
      unsigned width_s() const; // Sum (my be 0)

      Link& pin_Aclr();
      Link& pin_Clock();

      Link& pin_DataA(unsigned idx);
      Link& pin_DataB(unsigned idx);
      Link& pin_Result(unsigned idx);
      Link& pin_Sum(unsigned idx);

      const Link& pin_Aclr() const;
      const Link& pin_Clock() const;

      const Link& pin_DataA(unsigned idx) const;
      const Link& pin_DataB(unsigned idx) const;
      const Link& pin_Result(unsigned idx) const;
      const Link& pin_Sum(unsigned idx) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned width_r_;
      unsigned width_a_;
      unsigned width_b_;
      unsigned width_s_;
};


/*
 * This class represents an LPM_MUX device. This device has some
 * number of Result points (the width of the device) and some number
 * of input choices. There is also a selector of some width. The
 * parameters are:
 *
 *      width  -- Width of the result and each possible Data input
 *      size   -- Number of Data input (each of width)
 *      selw   -- Width in bits of the select input
 */
class NetMux  : public NetNode {

    public:
      NetMux(NetScope*scope, const string&n,
	     unsigned width, unsigned size, unsigned selw);
      ~NetMux();

      unsigned width() const;
      unsigned size() const;
      unsigned sel_width() const;

      Link& pin_Aclr();
      Link& pin_Clock();

      Link& pin_Result(unsigned);
      Link& pin_Data(unsigned wi, unsigned si);
      Link& pin_Sel(unsigned);

      const Link& pin_Aclr() const;
      const Link& pin_Clock() const;

      const Link& pin_Result(unsigned) const;
      const Link& pin_Data(unsigned, unsigned) const;
      const Link& pin_Sel(unsigned) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned width_;
      unsigned size_;
      unsigned swidth_;
};

/*
 * This device represents an LPM_RAM_DQ device. The actual content is
 * represented by a NetMemory object allocated elsewhere, but that
 * object fixes the width and size of the device. The pin count of the
 * address input is given in the constructor.
 */
class NetRamDq  : public NetNode {

    public:
      NetRamDq(NetScope*s, const string&name, NetMemory*mem, unsigned awid);
      ~NetRamDq();

      unsigned width() const;
      unsigned awidth() const;
      unsigned size() const;
      const NetMemory*mem() const;

      Link& pin_InClock();
      Link& pin_OutClock();
      Link& pin_WE();

      Link& pin_Address(unsigned idx);
      Link& pin_Data(unsigned idx);
      Link& pin_Q(unsigned idx);

      const Link& pin_InClock() const;
      const Link& pin_OutClock() const;
      const Link& pin_WE() const;

      const Link& pin_Address(unsigned idx) const;
      const Link& pin_Data(unsigned idx) const;
      const Link& pin_Q(unsigned idx) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

	// Use this method to absorb other NetRamDq objects that are
	// connected to the same memory, and have compatible pin
	// connections.
      void absorb_partners();

	// Use this method to count the partners (including myself)
	// that are ports to the attached memory.
      unsigned count_partners() const;

    private:
      NetMemory*mem_;
      NetRamDq*next_;
      unsigned awidth_;

};

/* =========
 * There are cases where expressions need to be represented. The
 * NetExpr class is the root of a heirarchy that serves that purpose.
 *
 * The expr_width() is the width of the expression, that accounts
 * for the widths of the sub-expressions I might have. It is up to the
 * derived classes to properly set the expr width, if need be. The
 * set_width() method is used to compel an expression to have a
 * certain width, and is used particulary when the expression is an
 * rvalue in an assignment statement.
 */
class NetExpr  : public LineInfo {
    public:
      explicit NetExpr(unsigned w =0);
      virtual ~NetExpr() =0;

      virtual void expr_scan(struct expr_scan_t*) const =0;
      virtual void dump(ostream&) const;

	// How wide am I?
      unsigned expr_width() const { return width_; }

	// Coerce the expression to have a specific width. If the
	// coersion works, then return true. Otherwise, return false.
      virtual bool set_width(unsigned);

	// This method returns true if the expression is
	// signed. Unsigned expressions return false.
      bool has_sign() const;
      void cast_signed(bool flag);

	// This returns true if the expression has a definite
	// width. This is generally true, but in some cases the
	// expression is amorphous and desires a width from its
	// environment. For example, 'd5 has indefinite width, but
	// 5'd5 has a definite width.

	// This method is only really used within concatenation
	// expressions to check validity.
      virtual bool has_width() const;


	// This method evaluates the expression and returns an
	// equivilent expression that is reduced as far as compile
	// time knows how. Essentially, this is designed to fold
	// constants.
      virtual NetExpr*eval_tree();

	// Make a duplicate of myself, and subexpressions if I have
	// any. This is a deep copy operation.
      virtual NetExpr*dup_expr() const =0;

	// Return a version of myself that is structural. This is used
	// for converting expressions to gates.
      virtual NetNet*synthesize(Design*);


    protected:
      void expr_width(unsigned w) { width_ = w; }

    private:
      unsigned width_;
      bool signed_flag_;

    private: // not implemented
      NetExpr(const NetExpr&);
      NetExpr& operator=(const NetExpr&);
};

/*
 * The expression constant is slightly special, and is sometimes
 * returned from other classes that can be evaluated at compile
 * time. This class represents constant values in expressions.
 */
class NetEConst  : public NetExpr {

    public:
      explicit NetEConst(const verinum&val);
      ~NetEConst();

      const verinum&value() const;

      virtual bool set_width(unsigned w);

      virtual bool has_width() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

      virtual NetEConst* dup_expr() const;
      virtual NetNet*synthesize(Design*);

    private:
      verinum value_;
};

/*
 * The NetTmp object is a network that is only used momentarily by
 * elaboration to carry links around. A completed netlist should not
 * have any of these within. This is a kind of wire, so it is NetNet
 * type. The constructor for this class also marks the NetNet as
 * local, so that it is not likely to suppress a real symbol.
 */
class NetTmp  : public NetNet {

    public:
      explicit NetTmp(NetScope*s, const string&name, unsigned npins =1);

};

/*
 * The NetBUFZ is a magic device that represents the continuous
 * assign, with the output being the target register and the input
 * the logic that feeds it. The netlist preserves the directional
 * nature of that assignment with the BUFZ. The target may elide it if
 * that makes sense for the technology.
 */
class NetBUFZ  : public NetNode {

    public:
      explicit NetBUFZ(NetScope*s, const string&n);
      ~NetBUFZ();

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
};

/*
 * This node is used to represent case equality in combinational
 * logic. Although this is not normally synthesizeable, it makes sense
 * to support an abstract gate that can compare x and z.
 *
 * This pins are assigned as:
 *
 *     0   -- Output (always returns 0 or 1)
 *     1   -- Input
 *     2   -- Input
 */
class NetCaseCmp  : public NetNode {

    public:
      explicit NetCaseCmp(NetScope*s, const string&n);
      ~NetCaseCmp();

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
};

/*
 * This class represents instances of the LPM_CONSTANT device. The
 * node has only outputs and a constant value. The width is available
 * by getting the pin_count(), and the value bits are available one at
 * a time. There is no meaning to the aggregation of bits to form a
 * wide NetConst object, although some targets may have an easier time
 * detecting interesting constructs if they are combined.
 */
class NetConst  : public NetNode {

    public:
      explicit NetConst(NetScope*s, const string&n, verinum::V v);
      explicit NetConst(NetScope*s, const string&n, const verinum&val);
      ~NetConst();

      verinum::V value(unsigned idx) const;

      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*, functor_t*);
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
      verinum::V*value_;
};

/*
 * This class represents all manner of logic gates. Pin 0 is OUTPUT and
 * all the remaining pins are INPUT. The BUFIF[01] gates have the
 * more specific pinout as follows:
 *
 *     bufif<N>
 *       0  -- output
 *       1  -- input data
 *       2  -- enable
 *
 * The pullup and pulldown gates have no inputs at all, and pin0 is
 * the output 1 or 0, depending on the gate type. It is the strength
 * of that value that is important.
 */
class NetLogic  : public NetNode {

    public:
      enum TYPE { AND, BUF, BUFIF0, BUFIF1, NAND, NMOS, NOR, NOT,
		  NOTIF0, NOTIF1, OR, PULLDOWN, PULLUP, RNMOS, RPMOS,
		  PMOS, XNOR, XOR };

      explicit NetLogic(NetScope*s, const string&n, unsigned pins, TYPE t);

      TYPE type() const { return type_; }

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*, functor_t*);

    private:
      const TYPE type_;
};

/*
 * The UDP is a User Defined Primitive from the Verilog source. Do not
 * expand it out any further then this in the netlist, as this can be
 * used to represent target device primitives.
 *
 * The UDP can be combinational or sequential. The sequential UDP
 * includes the current output in the truth table, and supports edges,
 * whereas the combinational does not and is entirely level sensitive.
 * In any case, pin 0 is an output, and all the remaining pins are
 * inputs.
 *
 * Set_table takes as input a string with one letter per pin. The
 * parser translates the written sequences to one of these. The
 * valid characters are:
 *
 *      0, 1, x  -- The levels
 *      r   -- (01)
 *      R   -- (x1)
 *      f   -- (10)
 *      F   -- (x0)
 *      P   -- (0x)
 *      N   -- (1x)
 *
 * It also takes one of the following glob letters to represent more
 * then one item.
 *
 *      p   -- 01, 0x or x1 // check this with the lexer
 *      n   -- 10, 1x or x0 // check this with the lexer
 *      ?   -- 0, 1, or x
 *      *   -- any edge
 *      +   -- 01 or x1
 *      _   -- 10 or x0  (Note that this is not the output '-'.)
 *      %   -- 0x or 1x
 *
 * SEQUENTIAL
 * These objects have a single bit of memory. The logic table includes
 * an entry for the current value, and allows edges on the inputs. In
 * canonical form, inly then entries that generate 0, 1 or - (no change)
 * are listed.
 *
 * COMBINATIONAL
 * The logic table is a map between the input levels and the
 * output. Each input pin can have the value 0, 1 or x and the output
 * can have the values 0 or 1. If the input matches nothing, the
 * output is x. In canonical form, only the entries that generate 0 or
 * 1 are listed.
 *
 */
#include "PUdp.h"

class NetUDP  : public NetNode {

    public:
      explicit NetUDP(NetScope*s, const string&n, unsigned pins, PUdp*u);

      virtual bool emit_node(struct target_t*) const;
      virtual void dump_node(ostream&, unsigned ind) const;

	/* Use these methods to scan the truth table of the
	   device. "first" returns the first item in the table, and
	   "next" returns the next item in the table. The method will
	   return false when the scan is done. */
      bool first(string&inp, char&out) const;
      bool next(string&inp, char&out) const;
      unsigned rows() const { return udp->tinput.count(); }

      unsigned nin() const { return pin_count()-1; }
      bool is_sequential() const { return udp->sequential; }
      string udp_name() const { return udp->name_; }
      char get_initial() const;

    private:
      mutable unsigned table_idx;
      PUdp *udp;
};


/* =========
 * A process is a behavioral-model description. A process is a
 * statement that may be compound. the various statement types may
 * refer to places in a netlist (by pointing to nodes) but is not
 * linked into the netlist. However, elaborating a process may cause
 * special nodes to be created to handle things like events.
 */
class NetProc : public LineInfo {

    public:
      explicit NetProc();
      virtual ~NetProc();

	// This method is called to emit the statement to the
	// target. The target returns true if OK, false for errors.
      virtual bool emit_proc(struct target_t*) const;

	// This method is called by functors that want to scan a
	// process in search of matchable patterns.
      virtual int match_proc(struct proc_match_t*);

      virtual void dump(ostream&, unsigned ind) const;

    private:
      friend class NetBlock;
      NetProc*next_;

    private: // not implemented
      NetProc(const NetProc&);
      NetProc& operator= (const NetProc&);
};

/*
 * Procedural assignment is broken into a suite of classes. These
 * classes represent the various aspects of the assignment statement
 * in behavioral code. (The continuous assignment is *not*
 * represented here.)
 *
 * The NetAssignBase carries the common aspects of an assignment,
 * including the r-value. This class has no cares of blocking vs
 * non-blocking, however it carries nearly all the other properties
 * of the assignment statement. It is abstract because it does not
 * differentiate the virtual behaviors.
 *
 * The NetAssign and NetAssignNB classes are the concrete classes that
 * give the assignment its final, precise meaning. These classes fill
 * in the NetProc behaviors.
 *
 * The l-value of the assignment is a collection of NetAssign_
 * objects that are connected to the structural netlist where the
 * assignment has its effect. The NetAssign_ class is not to be
 * derived from. 
 *
 * The collection is arranged from lsb up to msb, and represents the
 * concatenation of l-values. The elaborator may collapse some
 * concatenations into a single NetAssign_. The "more" member of the
 * NetAssign_ object points to the next most significant bits of l-value.
 *
 * NOTE: The elaborator will make an effort to match the width of the
 * r-value to the with of the l-value, but targets and functions
 * should know that this is not a guarantee.
 */

class NetAssign_ {

    public:
      NetAssign_(NetNet*sig);
      ~NetAssign_();

	// If this expression exists, then only a single bit is to be
	// set from the rval, and the value of this expression selects
	// the pin that gets the value.
      const NetExpr*bmux() const;

      unsigned get_loff() const;

      void set_bmux(NetExpr*);
      void set_part(unsigned loff, unsigned wid);

	// Get the width of the r-value that this node expects. This
	// method accounts for the presence of the mux, so it not
	// necessarily the same as the pin_count().
      unsigned lwidth() const;

	// Get the name of the underlying object.
      const char*name() const;

      NetNet* sig() const;

	// This pointer is for keeping simple lists.
      NetAssign_* more;

      void dump_lval(ostream&o) const;

    private:
      NetNet *sig_;
      NetExpr*bmux_;

      unsigned loff_;
      unsigned lwid_;
};

class NetAssignBase : public NetProc {

    public:
      NetAssignBase(NetAssign_*lv, NetExpr*rv);
      virtual ~NetAssignBase() =0;

	// This is the (procedural) value that is to be assigned when
	// the assignment is executed.
      NetExpr*rval();
      const NetExpr*rval() const;

      void set_rval(NetExpr*);

      NetAssign_* l_val(unsigned);
      const NetAssign_* l_val(unsigned) const;
      unsigned l_val_count() const;

	// This returns the total width of the accumulated l-value. It
	// accounts for any grouping of NetAssign_ objects that might happen.
      unsigned lwidth() const;

	// This dumps all the lval structures.
      void dump_lval(ostream&) const;

    private:
      NetAssign_*lval_;
      NetExpr   *rval_;
};

class NetAssign : public NetAssignBase {

    public:
      explicit NetAssign(NetAssign_*lv, NetExpr*rv);
      ~NetAssign();

      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;

    private:
};

class NetAssignNB  : public NetAssignBase {
    public:
      explicit NetAssignNB(NetAssign_*lv, NetExpr*rv);
      ~NetAssignNB();

      void rise_time(unsigned);
      void fall_time(unsigned);
      void decay_time(unsigned);

      unsigned rise_time() const;
      unsigned fall_time() const;
      unsigned decay_time() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;

    private:

      unsigned rise_time_;
      unsigned fall_time_;
      unsigned decay_time_;
};

/*
 * Assignment to memory is handled separately because memory is
 * not a node. There are blocking and non-blocking variants, just like
 * regular assign, and the NetAssignMem_ base class takes care of all
 * the common stuff.
 */
class NetAssignMem_ : public NetProc {

    public:
      explicit NetAssignMem_(NetMemory*, NetExpr*idx, NetExpr*rv);
      ~NetAssignMem_();

      NetMemory*memory() { return mem_; }
      NetExpr*index()    { return index_; }
      NetExpr*rval()     { return rval_; }

      const NetMemory*memory()const { return mem_; }
      const NetExpr*index()const    { return index_; }
      const NetExpr*rval()const     { return rval_; }

    private:
      NetMemory*mem_;
      NetExpr* index_;
      NetExpr* rval_;
};

class NetAssignMem : public NetAssignMem_ {

    public:
      explicit NetAssignMem(NetMemory*, NetExpr*idx, NetExpr*rv);
      ~NetAssignMem();

      virtual int match_proc(struct proc_match_t*);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
};

class NetAssignMemNB : public NetAssignMem_ {

    public:
      explicit NetAssignMemNB(NetMemory*, NetExpr*idx, NetExpr*rv);
      ~NetAssignMemNB();

      virtual int match_proc(struct proc_match_t*);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
};

/*
 * A block is stuff like begin-end blocks, that contain an ordered
 * list of NetProc statements.
 *
 * NOTE: The emit method calls the target->proc_block function but
 * does not recurse. It is up to the target-supplied proc_block
 * function to call emit_recurse.
 */
class NetBlock  : public NetProc {

    public:
      enum Type { SEQU, PARA };

      NetBlock(Type t) : type_(t), last_(0) { }
      ~NetBlock();

      const Type type() const { return type_; }

      void append(NetProc*);

      const NetProc*proc_first() const;
      const NetProc*proc_next(const NetProc*cur) const;

	// This version of emit_recurse scans all the statements of
	// the begin-end block sequentially. It is typically of use
	// for sequential blocks.
      void emit_recurse(struct target_t*) const;

      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;

    private:
      const Type type_;

      NetProc*last_;
};

/*
 * A CASE statement in the verilog source leads, eventually, to one of
 * these. This is different from a simple conditional because of the
 * way the comparisons are performed. Also, it is likely that the
 * target may be able to optimize differently.
 *
 * Case can be one of three types:
 *    EQ  -- All bits must exactly match
 *    EQZ -- z bits are don't care
 *    EQX -- x and z bits are don't care.
 */
class NetCase  : public NetProc {

    public:
      enum TYPE { EQ, EQX, EQZ };
      NetCase(TYPE c, NetExpr*ex, unsigned cnt);
      ~NetCase();

      void set_case(unsigned idx, NetExpr*ex, NetProc*st);

      TYPE type() const;
      const NetExpr*expr() const { return expr_; }
      unsigned nitems() const { return nitems_; }

      const NetExpr*expr(unsigned idx) const { return items_[idx].guard;}
      const NetProc*stat(unsigned idx) const { return items_[idx].statement; }

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:

      TYPE type_;

      struct Item {
	    NetExpr*guard;
	    NetProc*statement;
      };

      NetExpr* expr_;
      unsigned nitems_;
      Item*items_;
};

/*
 * The cassign statement causes the r-val net to be forced onto the
 * l-val reg when it is executed. The code generator is expected to
 * know what that means. All the expressions are structural and behave
 * like nets.
 *
 * This class is a NetProc because it it turned on by procedural
 * behavior. However, it is also a NetNode because it connects to
 * nets, and when activated follows the net values.
 */
class NetCAssign  : public NetProc, public NetNode {

    public:
      explicit NetCAssign(NetScope*s, const string&n, NetNet*l);
      ~NetCAssign();

      const Link& lval_pin(unsigned) const;

      virtual void dump(ostream&, unsigned ind) const;
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

      const NetNet*lval() const;

    private:
      NetNet*lval_;

    private: // not implemented
      NetCAssign(const NetCAssign&);
      NetCAssign& operator= (const NetCAssign&);
};


/* A condit represents a conditional. It has an expression to test,
   and a pair of statements to select from. */
class NetCondit  : public NetProc {

    public:
      explicit NetCondit(NetExpr*ex, NetProc*i, NetProc*e);
      ~NetCondit();

      const NetExpr*expr() const;
      NetExpr*expr();

      NetProc* if_clause();
      NetProc* else_clause();

	// Replace the condition expression.
      void set_expr(NetExpr*ex);

      bool emit_recurse_if(struct target_t*) const;
      bool emit_recurse_else(struct target_t*) const;

      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetExpr* expr_;
      NetProc*if_;
      NetProc*else_;
};

/*
 * The procedural deassign statement (the opposite of assign) releases
 * any assign expressions attached to the bits of the reg. The
 * lval is the expression of the "deassign <expr>;" statement with the
 * expr elaborated to a net.
 */
class NetDeassign : public NetProc {

    public:
      explicit NetDeassign(NetNet*l);
      ~NetDeassign();

      const NetNet*lval() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetNet*lval_;

    private: // not implemented
      NetDeassign(const NetDeassign&);
      NetDeassign& operator= (const NetDeassign&);
};

/*
 * This node represents the behavioral disable statement. The Verilog
 * source that produces it looks like:
 *
 *          disable <scope>;
 *
 * Where the scope is a named block or a task. It cannot be a module
 * instance scope because module instances cannot be disabled.
 */
class NetDisable  : public NetProc {

    public:
      explicit NetDisable(NetScope*tgt);
      ~NetDisable();

      const NetScope*target() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetScope*target_;

    private: // not implemented
      NetDisable(const NetDisable&);
      NetDisable& operator= (const NetDisable&);
};

/*
 * A NetEvent is an object that represents an event object, that is
 * objects declared like so in Verilog:
 *
 *        event foo;
 *
 * Once an object of this type exists, behavioral code can wait on the
 * event or trigger the event. Event waits refer to this object, as do
 * the event trigger statements. The NetEvent class may have a name and
 * a scope. The name is a simple name (no hierarchy) and the scope is
 * the NetScope that contains the object. The socpe member is written
 * by the NetScope object when the NetEvent is stored.
 *
 * The NetEvWait class represents a thread wait for an event. When
 * this statement is executed, it starts waiting on the
 * event. Conceptually, it puts itself on the event list for the
 * referenced event. When the event is triggered, the wit ends its
 * block and starts the associated statement.
 *
 * The NetEvTrig class represents trigger statements. Executing this
 * statement causes the referenced event to be triggered, which it
 * turn awakens the waiting threads. Each NetEvTrig object references
 * exactly one event object.
 *
 * The NetEvProbe class is the structural equivilent of the NetEvTrig,
 * in that it is a node and watches bit values that it receives. It
 * checks for edges then if appropriate triggers the associated
 * NetEvent. Each NetEvProbe references exactly one event object, and
 * the NetEvent objects have a list of NetEvProbe objects that
 * reference it.
 */
class NetEvent : public LineInfo {

      friend class NetScope;
      friend class NetEvProbe;
      friend class NetEvTrig;
      friend class NetEvWait;

    public:
      explicit NetEvent (const string&n);
      ~NetEvent();

      const char* name() const;
      string full_name() const;

	// Get information about probes connected to me.
      unsigned nprobe() const;
      NetEvProbe* probe(unsigned);
      const NetEvProbe* probe(unsigned) const;

	// Return the number of NetEvWait nodes that reference me.
      unsigned nwait() const;

      unsigned ntrig() const;

      NetScope* scope();
      const NetScope* scope() const;

	// Locate the first event that matches my behavior and
	// monitors the same signals.
      NetEvent* find_similar_event();

	// This method replaces pointers to me with pointers to
	// that. It is typically used to replace similar events
	// located by the find_similar_event method.
      void replace_event(NetEvent*that);

    private:
      char* name_;

	// The NetScope class uses these to list the events.
      NetScope*scope_;
      NetEvent*snext_;

	// Use these methods to list the probes attached to me.
      NetEvProbe*probes_;

	// Use these methods to list the triggers attached to me.
      NetEvTrig* trig_;

	// Use This member to count references by NetEvWait objects.
      unsigned waitref_;
      struct wcell_ {
	    NetEvWait*obj;
	    struct wcell_*next;
      };
      struct wcell_ *wlist_;

    private: // not implemented
      NetEvent(const NetEvent&);
      NetEvent& operator= (const NetEvent&);
};

class NetEvTrig  : public NetProc {

      friend class NetEvent;

    public:
      explicit NetEvTrig(NetEvent*tgt);
      ~NetEvTrig();

      const NetEvent*event() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetEvent*event_;
	// This is used to place me in the NetEvents lists of triggers.
      NetEvTrig*enext_;
};

class NetEvWait  : public NetProc {

    public:
      explicit NetEvWait(NetProc*st);
      ~NetEvWait();

      void add_event(NetEvent*tgt);
      void replace_event(NetEvent*orig, NetEvent*repl);

      unsigned nevents() const;
      const NetEvent*event(unsigned) const;
      NetEvent*event(unsigned);

      NetProc*statement();

      virtual bool emit_proc(struct target_t*) const;
      bool emit_recurse(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetProc*statement_;

      unsigned nevents_;
      NetEvent**events_;
};

class NetEvProbe  : public NetNode {

      friend class NetEvent;

    public:
      enum edge_t { ANYEDGE, POSEDGE, NEGEDGE };

      explicit NetEvProbe(NetScope*s, const string&n,
			  NetEvent*tgt, edge_t t, unsigned p);
      ~NetEvProbe();

      edge_t edge() const;
      NetEvent* event();
      const NetEvent* event() const;

      virtual bool emit_node(struct target_t*) const;
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
      NetEvent*event_;
      edge_t edge_;
	// The NetEvent class uses this to list me.
      NetEvProbe*enext_;
};

/*
 * The force statement causes the r-val net to be forced onto the
 * l-val net when it is executed. The code generator is expected to
 * know what that means. All the expressions are structural and behave
 * like nets.
 *
 * This class is a NetProc because it it turned on by procedural
 * behavior. However, it is also a NetNode because it connects to
 * nets, and when activated follows the net values.
 */
class NetForce  : public NetProc, public NetNode {

    public:
      explicit NetForce(NetScope*s, const string&n, NetNet*l);
      ~NetForce();

      const Link& lval_pin(unsigned) const;

      const NetNet*lval() const;

      virtual void dump(ostream&, unsigned ind) const;
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      NetNet*lval_;
};

/*
 * A forever statement is executed over and over again forever. Or
 * until its block is disabled.
 */
class NetForever : public NetProc {

    public:
      explicit NetForever(NetProc*s);
      ~NetForever();

      void emit_recurse(struct target_t*) const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetProc*statement_;
};

/*
 * A funciton definition is elaborated just like a task, though by now
 * it is certain that the first parameter (a phantom parameter) is the
 * output and all the remaining parameters are the inputs. This makes
 * for easy code generation in targets that support behavioral descriptions.
 */
class NetFuncDef {

    public:
      NetFuncDef(NetScope*, const svector<NetNet*>&po);
      ~NetFuncDef();

      void set_proc(NetProc*st);

      const string name() const;
      const NetProc*proc() const;
      NetScope*scope();

      unsigned port_count() const;
      const NetNet*port(unsigned idx) const;

      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetScope*scope_;
      NetProc*statement_;
      svector<NetNet*>ports_;
};

/*
 * This class represents delay statements of the form:
 *
 *     #<expr> <statement>
 *
 * Where the statement may be null. The delay is evaluated at
 * elaboration time to make a constant unsigned long that is the delay
 * in simulation ticks.
 *
 * If the delay expression is non-constant, construct the NetPDelay
 * object with a NetExpr* instead of the d value, and use th expr()
 * method to get the expression. If expr() returns 0, use the delay()
 * method to get the constant delay.
 */
class NetPDelay  : public NetProc {

    public:
      NetPDelay(unsigned long d, NetProc*st);
      NetPDelay(NetExpr* d, NetProc*st);
      ~NetPDelay();

      unsigned long delay() const;
      const NetExpr*expr() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

      bool emit_proc_recurse(struct target_t*) const;

    private:
      unsigned long delay_;
      NetExpr*expr_;
      NetProc*statement_;
};

/*
 * A repeat statement is executed some fixed number of times.
 */
class NetRepeat : public NetProc {

    public:
      explicit NetRepeat(NetExpr*e, NetProc*s);
      ~NetRepeat();

      const NetExpr*expr() const;
      void emit_recurse(struct target_t*) const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetExpr*expr_;
      NetProc*statement_;
};

/*
 * The procedural release statement (the opposite of force) releases
 * any force expressions attached to the bits of the wire or reg. The
 * lval is the expression of the "release <expr>;" statement with the
 * expr elaborated to a net.
 */
class NetRelease : public NetProc {

    public:
      explicit NetRelease(NetNet*l);
      ~NetRelease();

      const NetNet*lval() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetNet*lval_;
};


/*
 * The NetSTask class is a call to a system task. These kinds of tasks
 * are generally handled very simply in the target. They certainly are
 * handled differently from user defined tasks because ivl knows all
 * about the user defined tasks.
 */
class NetSTask  : public NetProc {

    public:
      NetSTask(const string&na, const svector<NetExpr*>&);
      ~NetSTask();

      const char* name() const;

      unsigned nparms() const;

      const NetExpr* parm(unsigned idx) const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      char* name_;
      svector<NetExpr*>parms_;
};

/*
 * This class represents an elaborated class definition. NetUTask
 * classes may refer to objects of this type to get the meaning of the
 * defined task.
 *
 * The task also introduces a scope, and the parameters are actually
 * reg objects in the new scope. The task is called by the calling
 * thread assigning (blocking assignment) to the in and inout
 * paramters, then invoking the thread, and finally assigning out the
 * output and inout variables. The variables accessable as ports are
 * also elaborated and accessible as ordinary reg objects.
 */
class NetTaskDef {

    public:
      NetTaskDef(const string&n, const svector<NetNet*>&po);
      ~NetTaskDef();

      void set_proc(NetProc*p);

      const string& name() const;
      const NetProc*proc() const;

      unsigned port_count() const;
      NetNet*port(unsigned idx);

      void dump(ostream&, unsigned) const;

    private:
      string name_;
      NetProc*proc_;
      svector<NetNet*>ports_;

    private: // not implemented
      NetTaskDef(const NetTaskDef&);
      NetTaskDef& operator= (const NetTaskDef&);
};

/*
 * This node represents a function call in an expression. The object
 * contains a pointer to the function definition, which is used to
 * locate the value register and input expressions.
 *
 * The NetNet parameter to the constructor is the *register* NetNet
 * that receives the result of the function, and the NetExpr list is
 * the paraneters passed to the function.
 */
class NetEUFunc  : public NetExpr {

    public:
      NetEUFunc(NetScope*, NetESignal*, svector<NetExpr*>&);
      ~NetEUFunc();

      const string name() const;

      const NetESignal*result() const;
      unsigned parm_count() const;
      const NetExpr* parm(unsigned idx) const;

      const NetScope* func() const;

      virtual bool set_width(unsigned);
      virtual void dump(ostream&) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEUFunc*dup_expr() const;

    private:
      NetScope*func_;
      NetESignal*result_;
      svector<NetExpr*> parms_;

    private: // not implemented
      NetEUFunc(const NetEUFunc&);
      NetEUFunc& operator= (const NetEUFunc&);
};

/*
 * A call to a user defined task is elaborated into this object. This
 * contains a pointer to the elaborated task definition, but is a
 * NetProc object so that it can be linked into statements.
 */
class NetUTask  : public NetProc {

    public:
      NetUTask(NetScope*);
      ~NetUTask();

      const string name() const;

      const NetScope* task() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetScope*task_;
};

/*
 * The while statement is a condition that is tested in the front of
 * each iteration, and a statement (a NetProc) that is executed as
 * long as the condition is true.
 */
class NetWhile  : public NetProc {

    public:
      NetWhile(NetExpr*c, NetProc*p)
      : cond_(c), proc_(p) { }

      const NetExpr*expr() const { return cond_; }

      void emit_proc_recurse(struct target_t*) const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetExpr* cond_;
      NetProc*proc_;
};


/*
 * The is the top of any process. It carries the type (initial or
 * always) and a pointer to the statement, probably a block, that
 * makes up the process.
 */
class NetProcTop  : public LineInfo {

    public:
      enum Type { KINITIAL, KALWAYS };

      NetProcTop(NetScope*s, Type t, class NetProc*st);
      ~NetProcTop();

      Type type() const { return type_; }
      NetProc*statement();
      const NetProc*statement() const;

      NetScope*scope();
      const NetScope*scope() const;

      void dump(ostream&, unsigned ind) const;
      bool emit(struct target_t*tgt) const;

    private:
      const Type type_;
      NetProc*const statement_;

      NetScope*scope_;
      friend class Design;
      NetProcTop*next_;
};

/*
 * This class represents a binary operator, with the left and right
 * operands and a single character for the operator. The operator
 * values are:
 *
 *   ^  -- Bit-wise exclusive OR
 *   +  -- Arithmetic add
 *   -  -- Arithmetic minus
 *   *  -- Arithmetic multiply
 *   /  -- Arithmetic divide
 *   %  -- Arithmetic modulus
 *   &  -- Bit-wise AND
 *   |  -- Bit-wise OR
 *   <  -- Less then
 *   >  -- Greater then
 *   e  -- Logical equality (==)
 *   E  -- Case equality (===)
 *   L  -- Less or equal
 *   G  -- Greater or equal
 *   n  -- Logical inequality (!=)
 *   N  -- Case inequality (!==)
 *   a  -- Logical AND (&&)
 *   o  -- Logical OR (||)
 *   O  -- Bit-wise NOR
 *   l  -- Left shift (<<)
 *   r  -- Right shift (>>)
 *   X  -- Bitwise exclusive NOR (~^)
 */
class NetEBinary  : public NetExpr {

    public:
      NetEBinary(char op, NetExpr*l, NetExpr*r);
      ~NetEBinary();

      const NetExpr*left() const { return left_; }
      const NetExpr*right() const { return right_; }

      char op() const { return op_; }

      virtual bool set_width(unsigned w);

	// A binary expression node only has a definite
	// self-determinable width if the operands both have definite
	// widths.
      virtual bool has_width() const;

      virtual NetEBinary* dup_expr() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    protected:
      char op_;
      NetExpr* left_;
      NetExpr* right_;

      virtual void eval_sub_tree_();
};

/*
 * The addition operators have slightly more complex width
 * calculations because there is the optional carry bit that can be
 * used. The operators covered by this class are:
 *   +  -- Arithmetic add
 *   -  -- Arithmetic minus
 */
class NetEBAdd : public NetEBinary {

    public:
      NetEBAdd(char op, NetExpr*l, NetExpr*r);
      ~NetEBAdd();

      virtual bool set_width(unsigned w);
      virtual NetEBAdd* dup_expr() const;
      virtual NetEConst* eval_tree();
      virtual NetNet* synthesize(Design*);
};

/*
 * This class represents the integer division operators.
 *   /  -- Divide
 *   %  -- Modulus
 */
class NetEBDiv : public NetEBinary {

    public:
      NetEBDiv(char op, NetExpr*l, NetExpr*r);
      ~NetEBDiv();

      virtual bool set_width(unsigned w);
      virtual NetEBDiv* dup_expr() const;
      virtual NetEConst* eval_tree();
      virtual NetNet* synthesize(Design*);
};

/*
 * The bitwise binary operators are represented by this class. This is
 * a specialization of the binary operator, so is derived from
 * NetEBinary. The particular constraints on these operators are that
 * operand and result widths match exactly, and each bit slice of the
 * operation can be represented by a simple gate. The operators
 * covered by this class are:
 *
 *   ^  -- Bit-wise exclusive OR
 *   &  -- Bit-wise AND
 *   |  -- Bit-wise OR
 *   O  -- Bit-wise NOR
 *   X  -- Bit-wise XNOR (~^)
 */
class NetEBBits : public NetEBinary {

    public:
      NetEBBits(char op, NetExpr*l, NetExpr*r);
      ~NetEBBits();

      virtual bool set_width(unsigned w);
      virtual NetEBBits* dup_expr() const;
      virtual NetEConst* eval_tree();

      virtual NetNet* synthesize(Design*);
};

/*
 * The binary comparison operators are handled by this class. This
 * this case the bit width of the expression is 1 bit, and the
 * operands take their natural widths. The supported operators are:
 *
 *   <  -- Less then
 *   >  -- Greater then
 *   e  -- Logical equality (==)
 *   E  -- Case equality (===)
 *   L  -- Less or equal (<=)
 *   G  -- Greater or equal (>=)
 *   n  -- Logical inequality (!=)
 *   N  -- Case inequality (!==)
 */
class NetEBComp : public NetEBinary {

    public:
      NetEBComp(char op, NetExpr*l, NetExpr*r);
      ~NetEBComp();

      virtual bool set_width(unsigned w);
      virtual NetEBComp* dup_expr() const;
      virtual NetEConst* eval_tree();

      virtual NetNet* synthesize(Design*);

    private:
      NetEConst*eval_eqeq_();
      NetEConst*eval_less_();
      NetEConst*eval_leeq_();
      NetEConst*eval_gt_();
      NetEConst*eval_gteq_();
      NetEConst*eval_neeq_();
      NetEConst*eval_eqeqeq_();
      NetEConst*eval_neeqeq_();
};

/*
 * The binary logical operators are those that return boolean
 * results. The supported operators are:
 *
 *   a  -- Logical AND (&&)
 *   o  -- Logical OR (||)
 */
class NetEBLogic : public NetEBinary {

    public:
      NetEBLogic(char op, NetExpr*l, NetExpr*r);
      ~NetEBLogic();

      virtual bool set_width(unsigned w);
      virtual NetEBLogic* dup_expr() const;
      virtual NetEConst* eval_tree();
      virtual NetNet* synthesize(Design*);

    private:
};


/*
 * Support the binary multiplication (*) operator.
 */
class NetEBMult : public NetEBinary {

    public:
      NetEBMult(char op, NetExpr*l, NetExpr*r);
      ~NetEBMult();

      virtual bool set_width(unsigned w);
      virtual NetEBMult* dup_expr() const;
      virtual NetEConst* eval_tree();

    private:
};


/*
 * The binary logical operators are those that return boolean
 * results. The supported operators are:
 *
 *   l  -- left shift (<<)
 *   r  -- right shift (>>)
 */
class NetEBShift : public NetEBinary {

    public:
      NetEBShift(char op, NetExpr*l, NetExpr*r);
      ~NetEBShift();

      virtual bool set_width(unsigned w);

	// A shift expression only needs the left expression to have a
	// definite width to give the expression a definite width.
      virtual bool has_width() const;

      virtual NetEBShift* dup_expr() const;
      virtual NetEConst* eval_tree();

    private:
};


/*
 * This expression node supports the concat expression. This is an
 * operator that just glues the results of many expressions into a
 * single value.
 *
 * Note that the class stores the parameter expressions in source code
 * order. That is, the parm(0) is placed in the most significant
 * position of the result.
 */
class NetEConcat  : public NetExpr {

    public:
      NetEConcat(unsigned cnt, unsigned repeat =1);
      ~NetEConcat();

	// Manipulate the parameters.
      void set(unsigned idx, NetExpr*e);

      unsigned repeat() const { return repeat_; }
      unsigned nparms() const { return parms_.count() ; }
      NetExpr* parm(unsigned idx) const { return parms_[idx]; }

      virtual bool set_width(unsigned w);
      virtual NetEConcat* dup_expr() const;
      virtual NetEConst*  eval_tree();
      virtual NetNet*synthesize(Design*);
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      svector<NetExpr*>parms_;
      unsigned repeat_;
};

/*
 * This clas is a placeholder for a parameter expression. When
 * parameters are first created, an instance of this object is used to
 * hold the place where the parameter exression goes. Then, when the
 * parameters are resolved, these objects are removed.
 *
 * If the parameter object is created with a path and name, then the
 * object represents a reference to a parameter that is known to exist.
 */
class NetEParam  : public NetExpr {
    public:
      NetEParam();
      NetEParam(class Design*des, NetScope*scope, const hname_t&name);
      ~NetEParam();

      virtual bool set_width(unsigned w);
      virtual bool has_width() const;
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetExpr* eval_tree();
      virtual NetEParam* dup_expr() const;

      virtual void dump(ostream&) const;

    private:
      Design*des_;
      NetScope*scope_;
      hname_t name_;
};


/*
 * This expression node supports bit/part selects from general
 * expressions. The sub-expression is self-sized, and has bits
 * selected from it. The base is the expression that identifies the
 * lsb of the expression, and the wid is the width of the part select,
 * or 1 for a bit select.
 */
class NetESelect  : public NetExpr {

    public:
      NetESelect(NetExpr*exp, NetExpr*base, unsigned wid);
      ~NetESelect();

      const NetExpr*sub_expr() const;
      const NetExpr*select() const;

      virtual bool set_width(unsigned w);
      virtual bool has_width() const;
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetESelect* dup_expr() const;

    private:
      NetExpr*expr_;
      NetExpr*base_;
};

/*
 * This class is a special (and magical) expression node type that
 * represents scope names. These can only be found as parameters to
 * NetSTask objects.
 */
class NetEScope  : public NetExpr {

    public:
      NetEScope(NetScope*);
      ~NetEScope();

      const NetScope* scope() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEScope* dup_expr() const;

      virtual void dump(ostream&os) const;

    private:
      NetScope*scope_;
};

/*
 * This node represents a system function call in an expression. The
 * object contains the name of the system function, which the backend
 * uses to do VPI matching.
 */
class NetESFunc  : public NetExpr {

    public:
      NetESFunc(const string&name, unsigned width, unsigned nprms);
      ~NetESFunc();

      const char* name() const;

      unsigned nparms() const;
      void parm(unsigned idx, NetExpr*expr);
      NetExpr* parm(unsigned idx);
      const NetExpr* parm(unsigned idx) const;

      virtual bool set_width(unsigned);
      virtual void dump(ostream&) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetESFunc*dup_expr() const;

    private:
      char* name_;
      unsigned nparms_;
      NetExpr**parms_;

    private: // not implemented
      NetESFunc(const NetESFunc&);
      NetESFunc& operator= (const NetESFunc&);
};

/*
 * This class represents the ternary (?:) operator. It has 3
 * expressions, one of which is a condition used to select which of
 * the other two expressions is the result.
 */
class NetETernary  : public NetExpr {

    public:
      NetETernary(NetExpr*c, NetExpr*t, NetExpr*f);
      ~NetETernary();

      virtual bool set_width(unsigned w);

      const NetExpr*cond_expr() const;
      const NetExpr*true_expr() const;
      const NetExpr*false_expr() const;

      virtual NetETernary* dup_expr() const;
      virtual NetExpr* eval_tree();

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;
      virtual NetNet*synthesize(Design*);

    private:
      NetExpr*cond_;
      NetExpr*true_val_;
      NetExpr*false_val_;
};

/*
 * This class represents a unary operator, with the single operand
 * and a single character for the operator. The operator values are:
 *
 *   ~  -- Bit-wise negation
 *   !  -- Logical negation
 *   &  -- Reduction AND
 *   |  -- Reduction OR
 *   ^  -- Reduction XOR
 *   +  --
 *   -  --
 *   A  -- Reduction NAND (~&)
 *   N  -- Reduction NOR (~|)
 *   X  -- Reduction NXOR (~^ or ^~)
 */
class NetEUnary  : public NetExpr {

    public:
      NetEUnary(char op, NetExpr*ex);
      ~NetEUnary();

      char op() const { return op_; }
      const NetExpr* expr() const { return expr_; }

      virtual bool set_width(unsigned w);

      virtual NetEUnary* dup_expr() const;
      virtual NetEConst* eval_tree();

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    protected:
      char op_;
      NetExpr* expr_;

      void eval_expr_();
};

class NetEUBits : public NetEUnary {

    public:
      NetEUBits(char op, NetExpr*ex);
      ~NetEUBits();

      virtual NetNet* synthesize(Design*);

      virtual NetEConst* eval_tree();
};

class NetEUReduce : public NetEUnary {

    public:
      NetEUReduce(char op, NetExpr*ex);
      ~NetEUReduce();

      virtual bool set_width(unsigned w);
      virtual NetNet* synthesize(Design*);
      virtual NetEConst* eval_tree();

};

/*
 * A reference to a memory is represented by this expression. If the
 * index is not supplied, then the node is only valid in certain
 * specific contexts.
 */
class NetEMemory  : public NetExpr {

    public:
      NetEMemory(NetMemory*mem, NetExpr*idx =0);
      virtual ~NetEMemory();

      const string& name () const;
      const NetExpr* index() const;

      virtual bool set_width(unsigned);

      NetExpr* eval_tree();
      virtual NetEMemory*dup_expr() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

      const NetMemory*memory() const { return mem_; };

    private:
      NetMemory*mem_;
      NetExpr* idx_;
};

/*
 * When a signal shows up in an expression, this type represents
 * it. From this the expression can get any kind of access to the
 * structural signal.
 *
 * A signal shows up as a node in the netlist so that structural
 * activity can invoke the expression. This node also supports part
 * select by indexing a range of the NetNet that is associated with
 * it. The msi() is the mose significant index, and lsi() the least
 * significant index.
 */
class NetESignal  : public NetExpr {

    public:
      NetESignal(NetNet*n);
      NetESignal(NetNet*n, unsigned msi, unsigned lsi);
      ~NetESignal();

      string name() const;
      virtual bool set_width(unsigned);

      virtual NetESignal* dup_expr() const;
      NetNet* synthesize(Design*des);

	// These methods actually reference the properties of the
	// NetNet object that I point to.
      unsigned bit_count() const;
      Link& bit(unsigned idx);

      const NetNet* sig() const;
      unsigned msi() const;
      unsigned lsi() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      NetNet*net_;
      unsigned msi_;
      unsigned lsi_;
};

/*
 * An expression that takes a bit of a signal is represented as
 * one of these. For example, ``foo[x+5]'' is a signal and x+5 is an
 * expression to select a single bit from that signal. I can't just
 * make a new NetESignal node connected to the single net because the
 * expression may vary during execution, so the structure is not known
 * at compile (elaboration) time.
 */
class NetEBitSel  : public NetExpr {

    public:
      NetEBitSel(NetESignal*sig, NetExpr*ex);
      ~NetEBitSel();

      string name() const;
      const NetExpr*index() const { return idx_; }

      virtual bool set_width(unsigned);

      const NetNet* sig() const;

      NetEBitSel* dup_expr() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
	// For now, only support single-bit selects of a signal.
      NetESignal*sig_;
      NetExpr* idx_;
};


/*
 * This object type is used to contain a logical scope within a
 * design. The scope doesn't represent any executable hardware, but is
 * just a handle that netlist processors can use to grab at the design.
 */
class NetScope {

    public:
      enum TYPE { MODULE, TASK, FUNC, BEGIN_END, FORK_JOIN };
      NetScope(NetScope*up, const char*name, TYPE t);
      ~NetScope();

	/* Parameters exist within a scope, and these methods allow
	   one to manipulate the set. In these cases, the name is the
	   *simple* name of the paramter, the heirarchy is implicit in
	   the scope. The return value from set_parameter is the
	   previous expression, if there was one. */

      NetExpr* set_parameter(const string&name, NetExpr*val);
      NetExpr* set_localparam(const string&name, NetExpr*val);
      const NetExpr*get_parameter(const string&name) const;

	/* These methods set or access events that live in this
	   scope. */

      void add_event(NetEvent*);
      void rem_event(NetEvent*);
      NetEvent*find_event(const hname_t&name);


	/* These methods manage signals. The add_ and rem_signal
	   methods are used by the NetNet objects to make themselves
	   available to the scope, and the find_signal method can be
	   used to locate signals within a scope. */

      void add_signal(NetNet*);
      void rem_signal(NetNet*);

      NetNet* find_signal(const string&name);
      NetNet* find_signal_in_child(const hname_t&name);


	/* ... and these methods manage memory the same way as signals
	   are managed above. */

      void add_memory(NetMemory*);
      void rem_memory(NetMemory*);

      NetMemory* find_memory(const string&name);


	/* The parent and child() methods allow users of NetScope
	   objects to locate nearby scopes. */
      NetScope* parent();
      NetScope* child(const string&name);
      const NetScope* parent() const;
      const NetScope* child(const string&name) const;

      TYPE type() const;

      void set_task_def(NetTaskDef*);
      void set_func_def(NetFuncDef*);
      void set_module_name(const char*);

      NetTaskDef* task_def();
      NetFuncDef* func_def();

      const NetTaskDef* task_def() const;
      const NetFuncDef* func_def() const;
      const char*module_name() const;

	/* Scopes have their own time units and time precision. The
	   unit and precision are given as power of 10, i.e. -3 is
	   units of milliseconds.

	   If a NetScope is created with a parent scope, the new scope
	   will initially inherit the unit and precision of the
	   parent scope. */

      void time_unit(int);
      void time_precision(int);

      int time_unit() const;
      int time_precision() const;

	/* The name of the scope is the fully qualified hierarchical
	   name, whereas the basename is just my name within my parent
	   scope. */
      const char* basename() const;
      string name() const;

      void run_defparams(class Design*);
      void evaluate_parameters(class Design*);

	/* This method generates a non-hierarchical name that is
	   guaranteed to be unique within this scope. */
      string local_symbol();
	/* This method generates a hierarchical name that is
	   guaranteed to be unique globally. */
      string local_hsymbol();

      void dump(ostream&) const;
      void emit_scope(struct target_t*tgt) const;
      void emit_defs(struct target_t*tgt) const;

	/* This method runs the functor on me. Recurse through the
	   children of this node as well. */
      void run_functor(Design*des, functor_t*fun);


	/* This member is used during elaboration to pass defparam
	   assignments from the scope pass to the parameter evaluation
	   step. After that, it is not used. */

      map<hname_t,NetExpr*>defparams;

    private:
      TYPE type_;
      char* name_;

      signed char time_unit_, time_prec_;

      map<string,NetExpr*>parameters_;
      map<string,NetExpr*>localparams_;

      NetEvent *events_;
      NetNet   *signals_;
      NetMemory*memories_;

      union {
	    NetTaskDef*task_;
	    NetFuncDef*func_;
	    char*module_name_;
      };

      NetScope*up_;
      NetScope*sib_;
      NetScope*sub_;

      unsigned lcounter_;
};

/*
 * This class contains an entire design. It includes processes and a
 * netlist, and can be passed around from function to function.
 */
class Design {

    public:
      Design();
      ~Design();


	/* The flags are a generic way of accepting command line
	   parameters/flags and passing them to the processing steps
	   that deal with the design. The compilation driver sets the
	   entire flags map after elaboration is done. Subsequent
	   steps can then use the get_flag() function to get the value
	   of an interesting key. */

      void set_flags(const map<string,string>&f) { flags_ = f; }

      string get_flag(const string&key) const;

      NetScope* make_root_scope(const char*name);
      NetScope* find_root_scope();
      list<NetScope*> find_root_scopes();

      const list<NetScope*> find_root_scopes() const;

	/* Attempt to set the precision to the specified value. If the
	   precision is already more precise, the keep the precise
	   setting. This is intended to hold the simulation precision
	   for use throughout the entire design. */

      void set_precision(int val);
      int  get_precision() const;

	/* This function takes a delay value and a scope, and returns
	   the delay value scaled to the precision of the design. */
      unsigned long scale_to_precision(unsigned long, const NetScope*)const;

	/* look up a scope. If no starting scope is passed, then the
	   path is taken as an absolute scope name. Otherwise, the
	   scope is located starting at the passed scope and working
	   up if needed. */
      NetScope* find_scope(const hname_t&path) const;
      NetScope* find_scope(NetScope*, const hname_t&path) const;

	// PARAMETERS

	/* This method searches for a parameter, starting in the given
	   scope. This method handles the upward searches that the
	   NetScope class itself does not support. */
      const NetExpr*find_parameter(const NetScope*, const hname_t&path) const;

      void run_defparams();
      void evaluate_parameters();

	/* This method locates a signal, starting at a given
	   scope. The name parameter may be partially hierarchical, so
	   this method, unlike the NetScope::find_signal method,
	   handles global name binding. */

      NetNet*find_signal(NetScope*scope, hname_t path);

	// Memories
      NetMemory* find_memory(NetScope*scope, hname_t path);

	/* This is a more general lookup that finds the named signal
	   or memory, whichever is first in the search path. */
      void find_symbol(NetScope*,const string&key,
		       NetNet*&sig, NetMemory*&mem);

	// Functions
      NetFuncDef* find_function(NetScope*scope, const hname_t&key);
      NetFuncDef* find_function(const hname_t&path);

	// Tasks
      NetScope* find_task(NetScope*scope, const hname_t&name);
      NetScope* find_task(const hname_t&key);

	// NODES
      void add_node(NetNode*);
      void del_node(NetNode*);

	// PROCESSES
      void add_process(NetProcTop*);
      void delete_process(NetProcTop*);

	// Iterate over the design...
      void dump(ostream&) const;
      void functor(struct functor_t*);
      bool emit(struct target_t*) const;

	// This is incremented by elaboration when an error is
	// detected. It prevents code being emitted.
      unsigned errors;

    public:
      string local_symbol(const string&path);

    private:
	// Keep a tree of scopes. The NetScope class handles the wide
	// tree and per-hop searches for me.
      list<NetScope*>root_scopes_;

	// List the nodes in the design
      NetNode*nodes_;

	// List the processes in the design.
      NetProcTop*procs_;
      NetProcTop*procs_idx_;

      map<string,string> flags_;

      int des_precision_;

      unsigned lcounter_;

    private: // not implemented
      Design(const Design&);
      Design& operator= (const Design&);
};


/* =======
 */

inline bool operator == (const Link&l, const Link&r)
{ return l.is_equal(r); }

inline bool operator != (const Link&l, const Link&r)
{ return ! l.is_equal(r); }

/* Connect the pins of two nodes together. Either may already be
   connected to other things, connect is transitive. */
extern void connect(Link&, Link&);

/* Return true if l and r are connected. */
inline bool connected(const Link&l, const Link&r)
{ return l.is_linked(r); }

/* return the number of links in the ring that are of the specified
   type. */
extern unsigned count_inputs(const Link&pin);
extern unsigned count_outputs(const Link&pin);
extern unsigned count_signals(const Link&pin);

/* Find the next link that is an output into the nexus. */
extern Link* find_next_output(Link*lnk);

/* Find the signal connected to the given node pin. There should
   always be exactly one signal. The bidx parameter get filled with
   the signal index of the Net, in case it is a vector. */
const NetNet* find_link_signal(const NetObj*net, unsigned pin,
			       unsigned&bidx);

inline ostream& operator << (ostream&o, const NetExpr&exp)
{ exp.dump(o); return o; }

extern ostream& operator << (ostream&, NetNet::Type);

/*
 * $Log: netlist.h,v $
 * Revision 1.231  2002/01/28 00:52:41  steve
 *  Add support for bit select of parameters.
 *  This leads to a NetESelect node and the
 *  vvp code generator to support that.
 *
 * Revision 1.230  2002/01/22 01:40:04  steve
 *  Precalculate constant results of memory index expressions.
 *
 * Revision 1.229  2002/01/19 19:02:08  steve
 *  Pass back target errors processing conditionals.
 *
 * Revision 1.228  2001/12/31 00:08:14  steve
 *  Support $signed cast of expressions.
 *
 * Revision 1.227  2001/12/03 04:47:15  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.226  2001/11/29 01:58:18  steve
 *  Handle part selects in l-values of DFF devices.
 *
 * Revision 1.225  2001/11/19 04:26:46  steve
 *  Unary reduction operators are all 1-bit results.
 *
 * Revision 1.224  2001/11/14 03:28:49  steve
 *  DLL target support for force and release.
 *
 * Revision 1.223  2001/11/09 03:43:26  steve
 *  Spelling errors.
 *
 * Revision 1.222  2001/11/08 05:15:51  steve
 *  Remove string paths from PExpr elaboration.
 *
 * Revision 1.221  2001/11/06 04:32:37  steve
 *  shift expressions can have definite widths.
 *
 * Revision 1.220  2001/10/31 05:24:52  steve
 *  ivl_target support for assign/deassign.
 *
 * Revision 1.219  2001/10/28 01:14:53  steve
 *  NetObj constructor finally requires a scope.
 *
 * Revision 1.218  2001/10/20 05:21:51  steve
 *  Scope/module names are char* instead of string.
 *
 * Revision 1.217  2001/10/19 21:53:24  steve
 *  Support multiple root modules (Philip Blundell)
 *
 * Revision 1.216  2001/10/16 02:19:27  steve
 *  Support IVL_LPM_DIVIDE for structural divide.
 *
 * Revision 1.215  2001/10/07 03:38:08  steve
 *  parameter names do not have defined size.
 *
 * Revision 1.214  2001/08/25 23:50:03  steve
 *  Change the NetAssign_ class to refer to the signal
 *  instead of link into the netlist. This is faster
 *  and uses less space. Make the NetAssignNB carry
 *  the delays instead of the NetAssign_ lval objects.
 *
 *  Change the vvp code generator to support multiple
 *  l-values, i.e. concatenations of part selects.
 *
 * Revision 1.213  2001/07/27 04:51:44  steve
 *  Handle part select expressions as variants of
 *  NetESignal/IVL_EX_SIGNAL objects, instead of
 *  creating new and useless temporary signals.
 *
 * Revision 1.212  2001/07/22 00:17:49  steve
 *  Support the NetESubSignal expressions in vvp.tgt.
 *
 * Revision 1.211  2001/07/04 22:59:25  steve
 *  handle left shifter in dll output.
 *
 * Revision 1.210  2001/07/01 00:27:34  steve
 *  Make NetFF constructor take const char* for the name.
 *
 * Revision 1.209  2001/06/16 23:45:05  steve
 *  Add support for structural multiply in t-dll.
 *  Add code generators and vvp support for both
 *  structural and behavioral multiply.
 *
 * Revision 1.208  2001/06/15 04:14:18  steve
 *  Generate vvp code for GT and GE comparisons.
 *
 * Revision 1.207  2001/06/07 02:12:43  steve
 *  Support structural addition.
 *
 * Revision 1.206  2001/05/08 23:59:33  steve
 *  Add ivl and vvp.tgt support for memories in
 *  expressions and l-values. (Stephan Boettcher)
 *
 * Revision 1.205  2001/04/29 20:19:10  steve
 *  Add pullup and pulldown devices.
 *
 * Revision 1.204  2001/04/24 02:23:58  steve
 *  Support for UDP devices in VVP (Stephen Boettcher)
 *
 * Revision 1.203  2001/04/22 23:09:46  steve
 *  More UDP consolidation from Stephan Boettcher.
 *
 * Revision 1.202  2001/04/06 02:28:02  steve
 *  Generate vvp code for functions with ports.
 *
 * Revision 1.201  2001/04/02 02:28:12  steve
 *  Generate code for task calls.
 *
 * Revision 1.200  2001/03/29 02:52:01  steve
 *  Add const probe method to NetEvent.
 *
 * Revision 1.199  2001/02/15 06:59:36  steve
 *  FreeBSD port has a maintainer now.
 *
 * Revision 1.198  2001/02/10 21:20:38  steve
 *  Binary operators with operands of indefinite width
 *  has itself an indefinite width.
 *
 * Revision 1.197  2001/02/10 20:29:39  steve
 *  In the context of range declarations, use elab_and_eval instead
 *  of the less robust eval_const methods.
 *
 * Revision 1.196  2001/02/09 05:44:23  steve
 *  support evaluation of constant < in expressions.
 *
 * Revision 1.195  2001/01/18 03:16:35  steve
 *  NetMux needs a scope. (PR#115)
 *
 * Revision 1.194  2001/01/16 02:44:18  steve
 *  Use the iosfwd header if available.
 *
 * Revision 1.193  2001/01/06 06:31:58  steve
 *  declaration initialization for time variables.
 *
 * Revision 1.192  2001/01/06 02:29:36  steve
 *  Support arrays of integers.
 *
 * Revision 1.191  2001/01/04 16:49:50  steve
 *  Evaluate constant === and !== expressions.
 *
 * Revision 1.190  2001/01/02 04:21:14  steve
 *  Support a bunch of unary operators in parameter expressions.
 *
 * Revision 1.189  2001/01/02 03:23:40  steve
 *  Evaluate constant &, | and unary ~.
 *
 * Revision 1.188  2000/12/16 19:03:30  steve
 *  Evaluate <= and ?: in parameter expressions (PR#81)
 *
 * Revision 1.187  2000/12/16 01:45:48  steve
 *  Detect recursive instantiations (PR#2)
 *
 * Revision 1.186  2000/12/11 00:31:43  steve
 *  Add support for signed reg variables,
 *  simulate in t-vvm signed comparisons.
 *
 * Revision 1.185  2000/12/05 06:29:33  steve
 *  Make signal attributes available to ivl_target API.
 *
 * Revision 1.184  2000/12/04 17:37:04  steve
 *  Add Attrib class for holding NetObj attributes.
 *
 * Revision 1.183  2000/12/02 05:08:04  steve
 *  Spelling error in comment.
 *
 * Revision 1.182  2000/11/29 05:24:00  steve
 *  synthesis for unary reduction ! and N operators.
 *
 * Revision 1.181  2000/11/29 02:09:53  steve
 *  Add support for || synthesis (PR#53)
 *
 * Revision 1.180  2000/11/20 00:58:40  steve
 *  Add support for supply nets (PR#17)
 *
 * Revision 1.179  2000/11/11 01:52:09  steve
 *  change set for support of nmos, pmos, rnmos, rpmos, notif0, and notif1
 *  change set to correct behavior of bufif0 and bufif1
 *  (Tim Leight)
 *
 *  Also includes fix for PR#27
 *
 * Revision 1.178  2000/11/11 00:03:36  steve
 *  Add support for the t-dll backend grabing flip-flops.
 *
 * Revision 1.177  2000/11/04 06:36:24  steve
 *  Apply sequential UDP rework from Stephan Boettcher  (PR#39)
 *
 * Revision 1.176  2000/10/31 17:49:02  steve
 *  Support time variables.
 *
 * Revision 1.175  2000/10/28 00:51:42  steve
 *  Add scope to threads in vvm, pass that scope
 *  to vpi sysTaskFunc objects, and add vpi calls
 *  to access that information.
 *
 *  $display displays scope in %m (PR#1)
 *
 * Revision 1.174  2000/10/18 20:04:39  steve
 *  Add ivl_lval_t and support for assignment l-values.
 *
 * Revision 1.173  2000/10/07 19:45:43  steve
 *  Put logic devices into scopes.
 *
 * Revision 1.172  2000/10/06 23:46:50  steve
 *  ivl_target updates, including more complete
 *  handling of ivl_nexus_t objects. Much reduced
 *  dependencies on pointers to netlist objects.
 *
 * Revision 1.171  2000/10/05 05:03:01  steve
 *  xor and constant devices.
 *
 * Revision 1.170  2000/10/04 16:30:39  steve
 *  Use char8 instead of string to store name.
 *
 * Revision 1.169  2000/09/29 04:43:09  steve
 *  Cnstant evaluation of NE.
 *
 * Revision 1.168  2000/09/26 05:05:58  steve
 *  Detect indefinite widths where definite widths are required.
 *
 * Revision 1.167  2000/09/26 01:35:42  steve
 *  Remove the obsolete NetEIdent class.
 *
 * Revision 1.166  2000/09/24 17:41:13  steve
 *  fix null pointer when elaborating undefined task.
 *
 * Revision 1.165  2000/09/24 15:44:44  steve
 *  Move some NetNet method out of the header file.
 *
 * Revision 1.164  2000/09/22 03:58:30  steve
 *  Access to the name of a system task call.
 *
 * Revision 1.163  2000/09/17 21:26:15  steve
 *  Add support for modulus (Eric Aardoom)
 *
 * Revision 1.162  2000/09/10 02:18:16  steve
 *  elaborate complex l-values
 *
 * Revision 1.161  2000/09/07 00:06:53  steve
 *  encapsulate access to the l-value expected width.
 *
 * Revision 1.160  2000/09/02 23:40:13  steve
 *  Pull NetAssign_ creation out of constructors.
 *
 * Revision 1.159  2000/09/02 20:54:20  steve
 *  Rearrange NetAssign to make NetAssign_ separate.
 *
 * Revision 1.158  2000/08/27 15:51:50  steve
 *  t-dll iterates signals, and passes them to the
 *  target module.
 *
 *  Some of NetObj should return char*, not string.
 *
 * Revision 1.157  2000/08/26 00:54:03  steve
 *  Get at gate information for ivl_target interface.
 *
 * Revision 1.156  2000/08/14 04:39:57  steve
 *  add th t-dll functions for net_const, net_bufz and processes.
 *
 * Revision 1.155  2000/08/09 03:43:45  steve
 *  Move all file manipulation out of target class.
 */
#endif
