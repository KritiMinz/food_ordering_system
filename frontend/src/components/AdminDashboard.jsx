import { useEffect, useState } from "react"

function AdminDashboard({ currentUser, onLogout }) {
  const [foods, setFoods] = useState([])
  const [showFoodForm, setShowFoodForm] = useState(false)
  const [editingFood, setEditingFood] = useState(null)
  const [message, setMessage] = useState("")
  const [error, setError] = useState("")
  const [orders, setOrders] = useState([])
const [loadingOrders, setLoadingOrders] = useState(false)
const [customers, setCustomers] = useState([])
const [loadingCustomers, setLoadingCustomers] = useState(false)

  const [form, setForm] = useState({
    name: "",
    description: "",
    price: "",
    category: "",
    image: "🍽️",
  })

  const loadFoods = async () => {
    try {
      const response = await fetch(
        "http://localhost:18080/api/foods"
      )

      const data = await response.json()

      if (!response.ok) {
        throw new Error("Failed to load foods")
      }

      setFoods(data)
    } catch (error) {
      setError(error.message)
    }
  }

  const loadOrders = async () => {
  try {
    setLoadingOrders(true)

    const response = await fetch(
      "http://localhost:18080/api/admin/orders"
    )

    if (!response.ok) {
      throw new Error("Failed to load orders")
    }

    const data = await response.json()

    setOrders(data)
  } catch (error) {
    setError(error.message)
  } finally {
    setLoadingOrders(false)
  }
}

const loadCustomers = async () => {
  try {
    setLoadingCustomers(true)

    const response = await fetch(
      "http://localhost:18080/api/admin/customers"
    )

    if (!response.ok) {
      throw new Error("Failed to load customers")
    }

    const data = await response.json()

    setCustomers(data)
  } catch (error) {
    setError(error.message)
  } finally {
    setLoadingCustomers(false)
  }
}

const updateOrderStatus = async (orderId, status) => {
  try {
    setError("")
    setMessage("")

    const response = await fetch(
      `http://localhost:18080/api/orders/${orderId}/status`,
      {
        method: "PUT",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          status,
        }),
      }
    )

    const data = await response.json()

    if (!response.ok) {
      throw new Error(
        data.message || "Failed to update order status"
      )
    }

    setOrders((currentOrders) =>
      currentOrders.map((order) =>
        order.id === orderId
          ? { ...order, status }
          : order
      )
    )

    setMessage(
      `Order #${orderId} status updated to ${status}`
    )
  } catch (error) {
    setError(error.message)
  }
}

  useEffect(() => {
  loadFoods()
  loadOrders()
  loadCustomers()
}, [])

  const resetForm = () => {
    setForm({
      name: "",
      description: "",
      price: "",
      category: "",
      image: "🍽️",
    })

    setEditingFood(null)
    setShowFoodForm(false)
  }

  const handleChange = (event) => {
    const { name, value } = event.target

    setForm((previous) => ({
      ...previous,
      [name]: value,
    }))
  }

  const handleSubmit = async (event) => {
    event.preventDefault()

    setMessage("")
    setError("")

    try {
      const url = editingFood
        ? `http://localhost:18080/api/admin/foods/${editingFood.id}`
        : "http://localhost:18080/api/admin/foods"

      const method = editingFood ? "PUT" : "POST"

      const response = await fetch(url, {
        method,
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          ...form,
          price: Number(form.price),
        }),
      })

      const data = await response.json()

      if (!response.ok) {
        throw new Error(
          data.message || "Failed to save food"
        )
      }

      setMessage(
        editingFood
          ? "Food updated successfully."
          : "Food added successfully."
      )

      resetForm()
      loadFoods()
    } catch (error) {
      setError(error.message)
    }
  }

  const handleEdit = (food) => {
    setEditingFood(food)

    setForm({
      name: food.name,
      description: food.description,
      price: food.price,
      category: food.category,
      image: food.image,
    })

    setShowFoodForm(true)
    setMessage("")
    setError("")
  }

  const handleDelete = async (foodId) => {
    const confirmed = window.confirm(
      "Are you sure you want to delete this food?"
    )

    if (!confirmed) {
      return
    }

    try {
      setMessage("")
      setError("")

      const response = await fetch(
        `http://localhost:18080/api/admin/foods/${foodId}`,
        {
          method: "DELETE",
        }
      )

      const data = await response.json()

      if (!response.ok) {
        throw new Error(
          data.message || "Failed to delete food"
        )
      }

      setMessage("Food deleted successfully.")

      loadFoods()
    } catch (error) {
      setError(error.message)
    }
  }

  if (!currentUser || currentUser.role !== "admin") {
    return (
      <section className="max-w-5xl mx-auto px-6 py-20">
        <div className="bg-white rounded-2xl shadow-md p-10 text-center">
          <div className="text-5xl mb-4">🔒</div>

          <h2 className="text-2xl font-bold text-gray-800">
            Admin Access Required
          </h2>

          <p className="text-gray-500 mt-2">
            You do not have permission to access this page.
          </p>
        </div>
      </section>
    )
  }

  return (
    <section className="max-w-7xl mx-auto px-6 py-10">

      {/* Header */}
      <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-4 mb-10">
        <div>
          <h1 className="text-4xl font-bold text-gray-800">
            Admin Dashboard
          </h1>

          <p className="text-gray-500 mt-2">
            Welcome, {currentUser.name}
          </p>
        </div>

        <button
          onClick={onLogout}
          className="bg-red-500 text-white px-5 py-2 rounded-lg hover:bg-red-600"
        >
          Logout
        </button>
      </div>

      {/* Messages */}
      {message && (
        <div className="bg-green-100 text-green-700 rounded-lg p-4 mb-6">
          {message}
        </div>
      )}

      {error && (
        <div className="bg-red-100 text-red-700 rounded-lg p-4 mb-6">
          {error}
        </div>
      )}

      {/* Dashboard cards */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-10">

        <div className="bg-white rounded-2xl shadow-md p-6">
          <div className="text-4xl mb-4">🍔</div>

          <h2 className="text-xl font-bold text-gray-800">
            Food Management
          </h2>

          <p className="text-gray-500 mt-2">
            Add, edit, delete and manage food items.
          </p>

          <button
            onClick={() => {
              setEditingFood(null)
              setShowFoodForm(true)
            }}
            className="mt-5 bg-orange-600 text-white px-5 py-2 rounded-lg hover:bg-orange-700"
          >
            Add Food
          </button>
        </div>

        <div className="bg-white rounded-2xl shadow-md p-6">
          <div className="text-4xl mb-4">📦</div>

          <h2 className="text-xl font-bold text-gray-800">
            Order Management
          </h2>

          <p className="text-gray-500 mt-2">
            View customer orders and update order status.
          </p>
        </div>

        <div className="bg-white rounded-2xl shadow-md p-6">
          <div className="text-4xl mb-4">👥</div>

          <h2 className="text-xl font-bold text-gray-800">
            Customers
          </h2>

          <p className="text-gray-500 mt-2">
            View registered customers and account information.
          </p>
        </div>

      </div>

      {/* Add/Edit Food Form */}
      {showFoodForm && (
        <div className="bg-white rounded-2xl shadow-md p-8 mb-10">

          <div className="flex justify-between items-center mb-6">
            <h2 className="text-2xl font-bold text-gray-800">
              {editingFood ? "Edit Food" : "Add New Food"}
            </h2>

            <button
              onClick={resetForm}
              className="text-gray-500 text-2xl hover:text-gray-800"
            >
              ×
            </button>
          </div>

          <form
            onSubmit={handleSubmit}
            className="grid grid-cols-1 md:grid-cols-2 gap-5"
          >

            <div>
              <label className="block font-semibold text-gray-700 mb-2">
                Food Name
              </label>

              <input
                name="name"
                value={form.name}
                onChange={handleChange}
                required
                className="w-full border rounded-lg px-4 py-3"
                placeholder="Chicken Burger"
              />
            </div>

            <div>
              <label className="block font-semibold text-gray-700 mb-2">
                Price
              </label>

              <input
                name="price"
                type="number"
                min="1"
                value={form.price}
                onChange={handleChange}
                required
                className="w-full border rounded-lg px-4 py-3"
                placeholder="120"
              />
            </div>

            <div>
              <label className="block font-semibold text-gray-700 mb-2">
                Category
              </label>

              <input
                name="category"
                value={form.category}
                onChange={handleChange}
                required
                className="w-full border rounded-lg px-4 py-3"
                placeholder="Burger"
              />
            </div>

            <div>
              <label className="block font-semibold text-gray-700 mb-2">
                Image
              </label>

              <input
                name="image"
                value={form.image}
                onChange={handleChange}
                className="w-full border rounded-lg px-4 py-3"
                placeholder="🍔"
              />
            </div>

            <div className="md:col-span-2">
              <label className="block font-semibold text-gray-700 mb-2">
                Description
              </label>

              <textarea
                name="description"
                value={form.description}
                onChange={handleChange}
                required
                rows="3"
                className="w-full border rounded-lg px-4 py-3"
                placeholder="Describe the food item..."
              />
            </div>

            <div className="md:col-span-2 flex gap-3">

              <button
                type="submit"
                className="bg-orange-600 text-white px-6 py-3 rounded-lg font-semibold hover:bg-orange-700"
              >
                {editingFood ? "Update Food" : "Add Food"}
              </button>

              <button
                type="button"
                onClick={resetForm}
                className="border border-gray-300 px-6 py-3 rounded-lg hover:bg-gray-100"
              >
                Cancel
              </button>

            </div>

          </form>
        </div>
      )}

      {/* =====================================================
    ORDER MANAGEMENT
===================================================== */}

<section className="bg-white rounded-2xl shadow p-6 mb-8">
  <div className="flex items-center justify-between mb-6">
    <div>
      <h2 className="text-2xl font-bold text-gray-800">
        📦 Order Management
      </h2>

      <p className="text-gray-500 mt-1">
        View customer orders and update order status.
      </p>
    </div>

    <button
      onClick={loadOrders}
      className="px-4 py-2 bg-gray-100 hover:bg-gray-200 rounded-lg font-medium"
    >
      🔄 Refresh
    </button>
  </div>

  {loadingOrders ? (
    <p className="text-gray-500">
      Loading orders...
    </p>
  ) : orders.length === 0 ? (
    <p className="text-gray-500">
      No orders found.
    </p>
  ) : (
    <div className="overflow-x-auto">
      <table className="w-full text-left">
        <thead>
          <tr className="border-b">
            <th className="py-3 px-3">Order</th>
            <th className="py-3 px-3">Customer</th>
            <th className="py-3 px-3">Items</th>
            <th className="py-3 px-3">Total</th>
            <th className="py-3 px-3">Payment</th>
            <th className="py-3 px-3">Address</th>
            <th className="py-3 px-3">Status</th>
          </tr>
        </thead>

        <tbody>
          {orders.map((order) => (
            <tr
              key={order.id}
              className="border-b hover:bg-gray-50"
            >
              <td className="py-4 px-3 font-semibold">
                #{order.id}
              </td>

              <td className="py-4 px-3">
                User #{order.userId}
              </td>

              <td className="py-4 px-3">
                <div className="space-y-1">
                  {order.items?.map((item) => (
                    <div key={item.id}>
                      {item.name} × {item.quantity}
                    </div>
                  ))}
                </div>
              </td>

              <td className="py-4 px-3 font-semibold">
                ₹{order.total}
              </td>

              <td className="py-4 px-3">
                {order.paymentMethod}
              </td>

              <td className="py-4 px-3">
                {order.address}
              </td>

              <td className="py-4 px-3">
                <select
                  value={order.status}
                  onChange={(event) =>
                    updateOrderStatus(
                      order.id,
                      event.target.value
                    )
                  }
                  className="border rounded-lg px-3 py-2 bg-white"
                >
                  <option value="PLACED">
                    PLACED
                  </option>

                  <option value="CONFIRMED">
                    CONFIRMED
                  </option>

                  <option value="PREPARING">
                    PREPARING
                  </option>

                  <option value="READY">
                    READY
                  </option>

                  <option value="OUT_FOR_DELIVERY">
                    OUT_FOR_DELIVERY
                  </option>

                  <option value="DELIVERED">
                    DELIVERED
                  </option>

                  <option value="CANCELLED">
                    CANCELLED
                  </option>
                </select>
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  )}
</section>

{/* =====================================================
    CUSTOMER MANAGEMENT
===================================================== */}

<section className="bg-white rounded-2xl shadow p-6 mb-8">
  <div className="flex items-center justify-between mb-6">
    <div>
      <h2 className="text-2xl font-bold text-gray-800">
        👥 Customer Management
      </h2>

      <p className="text-gray-500 mt-1">
        View registered customer accounts.
      </p>
    </div>

    <button
      onClick={loadCustomers}
      className="px-4 py-2 bg-gray-100 hover:bg-gray-200 rounded-lg font-medium"
    >
      🔄 Refresh
    </button>
  </div>

  <div className="mb-5">
    <div className="inline-block bg-gray-100 rounded-xl px-5 py-3">
      <span className="text-gray-500">
        Total Customers
      </span>

      <span className="ml-3 text-2xl font-bold">
        {customers.length}
      </span>
    </div>
  </div>

  {loadingCustomers ? (
    <p className="text-gray-500">
      Loading customers...
    </p>
  ) : customers.length === 0 ? (
    <p className="text-gray-500">
      No customers registered.
    </p>
  ) : (
    <div className="overflow-x-auto">
      <table className="w-full text-left">
        <thead>
          <tr className="border-b">
            <th className="py-3 px-3">
              ID
            </th>

            <th className="py-3 px-3">
              Name
            </th>

            <th className="py-3 px-3">
              Phone
            </th>

            <th className="py-3 px-3">
              Email
            </th>

            <th className="py-3 px-3">
              Role
            </th>
          </tr>
        </thead>

        <tbody>
          {customers.map((customer) => (
            <tr
              key={customer.id}
              className="border-b hover:bg-gray-50"
            >
              <td className="py-4 px-3">
                #{customer.id}
              </td>

              <td className="py-4 px-3 font-semibold">
                {customer.name}
              </td>

              <td className="py-4 px-3">
                {customer.phone}
              </td>

              <td className="py-4 px-3">
                {customer.email}
              </td>

              <td className="py-4 px-3">
                <span className="px-3 py-1 rounded-full bg-gray-100 text-sm">
                  {customer.role}
                </span>
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  )}
</section>

      {/* Food List */}
      <div className="bg-white rounded-2xl shadow-md overflow-hidden">

        <div className="p-6 border-b">
          <div className="flex items-center justify-between">
            <div>
              <h2 className="text-2xl font-bold text-gray-800">
                Food Items
              </h2>

              <p className="text-gray-500 mt-1">
                {foods.length} food items available
              </p>
            </div>

            <button
              onClick={loadFoods}
              className="border border-orange-600 text-orange-600 px-4 py-2 rounded-lg hover:bg-orange-50"
            >
              Refresh
            </button>
          </div>
        </div>

        <div className="overflow-x-auto">

          <table className="w-full">

            <thead className="bg-gray-50">
              <tr>
                <th className="text-left px-6 py-4">
                  Food
                </th>

                <th className="text-left px-6 py-4">
                  Category
                </th>

                <th className="text-left px-6 py-4">
                  Price
                </th>

                <th className="text-right px-6 py-4">
                  Actions
                </th>
              </tr>
            </thead>

            <tbody>

              {foods.map((food) => (
                <tr
                  key={food.id}
                  className="border-t"
                >

                  <td className="px-6 py-4">

                    <div className="flex items-center gap-4">

                      <div className="text-4xl">
                        {food.image}
                      </div>

                      <div>
                        <p className="font-bold text-gray-800">
                          {food.name}
                        </p>

                        <p className="text-sm text-gray-500">
                          {food.description}
                        </p>
                      </div>

                    </div>

                  </td>

                  <td className="px-6 py-4 text-gray-600">
                    {food.category}
                  </td>

                  <td className="px-6 py-4 font-bold text-orange-600">
                    ₹{food.price}
                  </td>

                  <td className="px-6 py-4">

                    <div className="flex justify-end gap-2">

                      <button
                        onClick={() => handleEdit(food)}
                        className="border border-blue-500 text-blue-600 px-4 py-2 rounded-lg hover:bg-blue-50"
                      >
                        Edit
                      </button>

                      <button
                        onClick={() => handleDelete(food.id)}
                        className="border border-red-500 text-red-600 px-4 py-2 rounded-lg hover:bg-red-50"
                      >
                        Delete
                      </button>

                    </div>

                  </td>

                </tr>
              ))}

            </tbody>

          </table>

        </div>
      </div>

      {/* System Information */}
      <div className="bg-white rounded-2xl shadow-md p-6 mt-8">

        <h2 className="text-2xl font-bold text-gray-800 mb-4">
          System Information
        </h2>

        <div className="grid grid-cols-1 md:grid-cols-3 gap-4">

          <div className="bg-orange-50 rounded-lg p-4">
            <p className="text-gray-500">Backend</p>
            <p className="font-bold text-orange-600">
              C++17
            </p>
          </div>

          <div className="bg-orange-50 rounded-lg p-4">
            <p className="text-gray-500">Storage</p>
            <p className="font-bold text-orange-600">
              JSON Files
            </p>
          </div>

          <div className="bg-orange-50 rounded-lg p-4">
            <p className="text-gray-500">Server</p>
            <p className="font-bold text-orange-600">
              Crow HTTP Server
            </p>
          </div>

        </div>

      </div>

    </section>
  )
}

export default AdminDashboard