#include <stddef.h>
#include "edge.h"

void Jarvis::Edge::init(Node &src, Node &dest, StringID tag, unsigned obj_size)
{
    _src = &src;
    _dest = &dest;
    _tag = tag;
    _property_list.init(obj_size - offsetof(Edge, _property_list));
}

bool Jarvis::Edge::check_property(StringID id, Property &result) const
    { return _property_list.check_property(id, result); }

Jarvis::Property Jarvis::Edge::get_property(StringID id) const
    { return _property_list.get_property(id); }

Jarvis::PropertyIterator Jarvis::Edge::get_properties() const
    { return _property_list.get_properties(); }

void Jarvis::Edge::set_property(StringID id, const Property &p)
    { _property_list.set_property(id, p); }

void Jarvis::Edge::remove_property(StringID id)
    { _property_list.remove_property(id); }